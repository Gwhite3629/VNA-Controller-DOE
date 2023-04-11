#include "GPIB_prof.h"
#include "commands.h"
#include "file.h"
#include "serial.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <windows.h>
// Error handle macro
#define handle_error(ret, msg) \
  do                           \
  {                            \
    errno = ret;               \
    perror(msg);               \
    exit(EXIT_FAILURE);        \
  } while (0)

struct args
{
  FILE *profdat;
  fpos_t *prof_start;
  fpos_t *prof_end;
  HANDLE fd;
  int profile;
  int points;
  int channum;
  char *chan[4];
  char *testtype;
  int interval;
  bool time_flag;
  int testtime;
  int cal;
  int fstar;
  int fstop;
  int fc;
};
// Thread concurrency stuff
atomic_bool exit_flag = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t wait = PTHREAD_MUTEX_INITIALIZER;
// Simple GPIB selector
int GPIB_sel(HANDLE fd, int profile)
{
  int ret;

  switch (profile)
  {
  case 0:
    ret = def(fd);
    if (ret < 0)
      goto fail;
  }
  return 0;
fail:
  return -1;
}
// Prints filecount
static void cleanup_handler(void *arg)
{
  int *fc = (int *)arg;
  printf("File Count:%d\n", *fc);
}
// Frees any argument
static void free_indirect(void *arg)
{
  free(*((void **)arg));
}

static void *dat_col(void *arg)
{
  struct args *prof = (struct args *)arg;
  char *dat = NULL;
  char *buff = NULL;
  time_t start_t = 0;
  time_t cur_t = 0;
  int ret = 0;
  int i;
  time_t filet;
  // Timing and filecount preparation
  struct timespec timeout;
  clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_sec += (60 * prof->interval);
  pthread_cleanup_push(cleanup_handler, &prof->fc);
  pthread_cleanup_push(free_indirect, &dat);
  pthread_cleanup_push(free_indirect, &buff);
  // Allocation of arrays and memset to 0
  dat = malloc(50 * prof->points);
  if (dat == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(dat, 0, 50 * prof->points);

  buff = malloc(2);
  if (buff == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(buff, 0, 2);
  // Timing and preset machine
  time(&start_t);
  time(&cur_t);
  ret = write_port(prof->fd, "OPC?;PRES;\r", 11);
  if (ret < 0)
    goto fail;
  ret = read_port(prof->fd, buff, 2);
  if (ret < 0)
  {
    goto fail;
  }
  // Conditionally data gathering
  switch (prof->time_flag)
  {
    // Indefinite run
  case 0:
    while (!exit_flag)
    {
      // Cancel point
      pthread_testcancel();
      // Get data
      for (i = 0; i < prof->channum; i++)
      {
        if (prof->chan[i] != NULL)
        {
          ret = get_DATA(prof->fd, dat, prof->points, prof->chan[i], prof->fstar,
                         prof->fstop, &prof->fc, prof->cal, &filet);
          if (ret < 0)
            goto fail;
          ret = catFile(filet, prof->chan[i], prof->fstar, prof->fstop, prof->points);
          if (ret < 0)
            goto fail;
        }
      }
      // Lock thread variable
      pthread_mutex_lock(&wait);
      // Wait for condition break or timeout or unlock
      pthread_cond_timedwait(&cond, &wait, &timeout);
      // Unlock
      pthread_mutex_unlock(&wait);
      // Increment timeout
      timeout.tv_sec += (60 * prof->interval);
    }
    break;
    // Timed run
  case 1:
    // Identical to above with a time constraint
    while ((difftime(cur_t, start_t) < (60 * prof->testtime)) && (!(exit_flag)))
    {
      pthread_testcancel();
      for (i = 0; i < prof->channum; i++)
      {
        if (prof->chan[i] != NULL)
        {
          ret = get_DATA(prof->fd, dat, prof->points, prof->chan[i], prof->fstar,
                         prof->fstop, &prof->fc, prof->cal, &filet);
          if (ret < 0)
            goto fail;
          ret = catFile(filet, prof->chan[i], prof->fstar, prof->fstop, prof->points);
          if (ret < 0)
            goto fail;
        }
      }
      pthread_mutex_lock(&wait);
      pthread_cond_timedwait(&cond, &wait, &timeout);
      pthread_mutex_unlock(&wait);
      timeout.tv_sec += (60 * prof->interval);
      // Get current time
      time(&cur_t);
    }
    break;
  }
// Cleanups are run anyway, this just ensures proper execution
fail:
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(1);
  pthread_cleanup_pop(prof->fc);
  pthread_exit(NULL);
  return 0;
}
// Interpret user profile
int prep_args(struct args **prof, HANDLE fd)
{
  // Allocate for structure and memset
  int i;
  *prof = malloc(sizeof(struct args));
  if (prof == NULL)
    goto fail;
  memset((*prof), 0, sizeof(struct args));
  (*prof)->prof_start = malloc(sizeof(fpos_t));
  if ((*prof)->prof_start == NULL)
    goto fail;
  memset((*prof)->prof_start, 0, sizeof(fpos_t));

  (*prof)->prof_end = malloc(sizeof(fpos_t));
  if ((*prof)->prof_end == NULL)
    goto fail;
  memset((*prof)->prof_end, 0, sizeof(fpos_t));

  (*prof)->chan[0] = malloc(8);
  if ((*prof)->chan == NULL)
    goto fail;
  memset((*prof)->chan[0], 0, 8);
  (*prof)->chan[1] = malloc(8);
  if ((*prof)->chan == NULL)
    goto fail;
  memset((*prof)->chan[1], 0, 8);
  (*prof)->chan[2] = malloc(8);
  if ((*prof)->chan == NULL)
    goto fail;
  memset((*prof)->chan[2], 0, 8);
  (*prof)->chan[3] = malloc(8);
  if ((*prof)->chan == NULL)
    goto fail;
  memset((*prof)->chan[3], 0, 8);

  (*prof)->testtype = malloc(8);
  if ((*prof)->testtype == NULL)
    goto fail;
  memset((*prof)->testtype, 0, 8);

  (*prof)->fd = fd;
  (*prof)->fc = 0;

  printf("Select profile:");
  scanf("%d", &(*prof)->profile);

  (*prof)->profdat = fopen("profiles_data", "r");
  if ((*prof)->profdat == NULL)
    goto fail;
  find_prof((*prof)->profdat, (*prof)->profile, (*prof)->prof_start,
            (*prof)->prof_end);

  fsetpos((*prof)->profdat, (*prof)->prof_start);
  // Gets all profile data and fills structure
  fscanf((*prof)->profdat, "%d", &(*prof)->points);
  fscanf((*prof)->profdat, "%d", &(*prof)->channum);
  for (i = 0; i < 4; i++)
  {
    if (i < (*prof)->channum)
    {
      fscanf((*prof)->profdat, "%s", (*prof)->chan[i]);
    }
    else
    {
      (*prof)->chan[i] = NULL;
    }
  }
  fscanf((*prof)->profdat, "%s", (*prof)->testtype);
  fscanf((*prof)->profdat, "%d", &(*prof)->interval);
  if (strcmp((*prof)->testtype, "TIMED") == 0)
  {
    (*prof)->time_flag = 1;
    fscanf((*prof)->profdat, "%d", &(*prof)->testtime);
  }
  fscanf((*prof)->profdat, "%d", &(*prof)->cal);
  fscanf((*prof)->profdat, "%d", &(*prof)->fstar);
  fscanf((*prof)->profdat, "%d", &(*prof)->fstop);
  // Prints profile to ensure correct selection
  printf("points:%d\n", (*prof)->points);
  printf("channum:%d\n", (*prof)->channum);
  for (i = 0; i < (*prof)->channum; i++)
  {
    printf("chan:%s\n", (*prof)->chan[i]);
  }
  printf("testtype:%s\n", (*prof)->testtype);
  printf("interval:%d\n", (*prof)->interval);
  if ((*prof)->time_flag)
    printf("testtime:%d\n", (*prof)->testtime);
  printf("calk:%d\n", (*prof)->cal);
  printf("fstar:%d\n", (*prof)->fstar);
  printf("fstop:%d\n", (*prof)->fstop);

  fclose((*prof)->profdat);
  return 1;

fail:
  fclose((*prof)->profdat);
  if (i = 0; i < 4; i++)
  {
    free((*prof)->chan[i]);
  }
  free((*prof)->prof_start);
  free((*prof)->prof_end);
  free((*prof)->testtype);
  free((prof));
  return -1;
}
// Main function for handling
int DATA_sel(HANDLE fd)
{
  int ret = 0;
  pthread_t thr;
  char *detect = NULL;
  char *buff = NULL;
  struct args *prof = NULL;
  ret = prep_args(&prof, fd);
  if (ret < 0)
    goto fail;
  // Create and begin thread execution
  ret = pthread_create(&thr, NULL, &dat_col, (void *)prof);
  if (ret != 0)
    handle_error(ret, "pthread_create");
  // Allocate and memset to 0
  detect = malloc(64);
  if (detect == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(detect, 0, 64);
  buff = malloc(2);
  if (buff == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(buff, 0, 2);
  // Wait for user to cancel
  while (!exit_flag)
  {
    scanf("%s", detect);
    if (strcmp(detect, "quit") == 0)
    {
      exit_flag = 1;
      pthread_cond_broadcast(&cond);
      ret = pthread_cancel(thr);
      if (ret != 0)
        handle_error(ret, "pthread_cancel");
    }
  }
  // Exit thread
  pthread_join(thr, NULL);

  ret = write_port(fd, "OPC?;WAIT;\r", 11);
  if (ret < 0)
    goto fail;
  ret = read_port(fd, buff, 2);
  if (ret < 0)
    goto fail;

  free(buff);
  if (i = 0; i < 4; i++)
  {
    free((prof->chan[i]));
  }
  free((prof)->prof_start);
  free((prof)->prof_end);
  free((prof)->testtype);
  free((prof));
  free(detect);
  exit(EXIT_SUCCESS);

fail:
  free(buff);
  if (i = 0; i < 4; i++)
  {
    free((prof->chan[i]));
  }
  free((prof)->prof_start);
  free((prof)->prof_end);
  free((prof)->testtype);
  free((prof));
  free(detect);
  exit(EXIT_FAILURE);
}
