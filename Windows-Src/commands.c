#include "serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

char *calk[11] = {
    "CALK24MM;\r",
    "CALK292MM;\r",
    "CALK292S;\r",
    "CALK35MD;\r",
    "CALK35MC;\r",
    "CALK716;\r",
    "CALK7MM;\r",
    "CALKN50;\r",
    "CALKN75;\r",
    "CALKTRLK;\r",
    "CALKUSED;\r"};

// This function gets the data from the host machine and adds it to the file
int get_DATA(HANDLE fd, char *dat, int points, char *channel, int fstar,
             int fstop, int *fc, int cal, time_t *filet)
{
  FILE *f;
  char *file = NULL;
  int ret;
  int i;
  char *channel_com = NULL;
  char *point_com = NULL;
  char *freq_com = NULL;
  char *buff = NULL;
  *filet = time(NULL);
  // Allocate all arrays and set them to 0
  file = malloc(64);
  if (file == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(file, 0, 64);
  channel_com = malloc(8);
  if (channel_com == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(channel_com, 0, 8);
  point_com = malloc(12);
  if (point_com == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(point_com, 0, 12);
  freq_com = malloc(16);
  if (freq_com == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(freq_com, 0, 16);
  buff = malloc(4);
  if (buff == NULL)
  {
    perror("memory error");
    goto fail;
  }
  memset(buff, 0, 4);
  // Prepare used based commands
  sprintf(freq_com, "STAR%dMHZ;\r", fstar);
  ret = write_port(fd, freq_com, strlen(freq_com));
  if (ret < 0)
    goto fail;
  sprintf(freq_com, "STOP%dMHZ;\r", fstop);
  ret = write_port(fd, freq_com, strlen(freq_com));
  if (ret < 0)
    goto fail;

  sprintf(point_com, "POIN%d;\r", points);
  sprintf(channel_com, "%s;\r", channel);
  // Write all sequential commands
  ret = write_port(fd, point_com, strlen(point_com));
  if (ret < 0)
    goto fail;

  ret = write_port(fd, "CORRON;\r", 8);
  if (ret < 0)
    goto fail;

  ret = write_port(fd, calk[cal], strlen(calk[cal]));
  if (ret < 0)
    goto fail;

  ret = write_port(fd, "CHAN1;\r", 7);
  if (ret < 0)
    goto fail;

  ret = write_port(fd, channel_com, 5);
  if (ret < 0)
    goto fail;

  ret = write_port(fd, "LOGM;\r", 6);
  if (ret < 0)
    goto fail;

  ret = write_port(fd, "OPC?;SING;\r", 11);
  if (ret < 0)
    goto fail;
  ret = read_port(fd, buff, 4);
  if (ret < 0)
    goto fail;

  ret = write_port(fd, "FORM4;\r", 7);
  if (ret < 0)
    goto fail;

  ret = write_port(fd, "OUTPFORM;\r", 10);
  if (ret < 0)
    goto fail;
  ret = read_port(fd, dat, 50 * points);
  if (ret < 0)
    goto fail;
  ret = sprintf(file, "data%smagTemp%s", channel, ctime(filet));
  if (ret < 0)
    goto fail;

  // Open file and write data buffer to it
  f = fopen(file, "w");
  if (f == NULL)
  {
    perror("failed to open file");
    fclose(f);
    goto fail;
  }

  for (i = 0; i < (50 * points); i++)
  {
    ret = fprintf(f, "%c", dat[i]);
    if (ret < 0)
    {
      fclose(f);
      goto fail;
    }
  }

  fclose(f);

  ret = write_port(fd, "CHAN2;\r", 7);
  if (ret < 0)
    goto fail;
  // Tells the machine what channel to measure on
  ret = write_port(fd, channel_com, 5);
  if (ret < 0)
    goto fail;
  ret = write_port(fd, "PHAS;\r", 6);
  if (ret < 0)
    goto fail;
  // Performs a single sweep on the gathered data and flushes to machine memory
  ret = write_port(fd, "OPC?;SING;\r", 11);
  if (ret < 0)
    goto fail;
  ret = read_port(fd, buff, 4);
  if (ret < 0)
    goto fail;
  // Output format, 64bit Floating point
  ret = write_port(fd, "FORM4;\r", 7);
  if (ret < 0)
    goto fail;
  // Output corrected data in real imaginary pairs
  ret = write_port(fd, "OUTPFORM;\r", 10);
  if (ret < 0)
    goto fail;
  ret = read_port(fd, dat, 50 * points);
  if (ret < 0)
    goto fail;

  // File preparation
  // Removes file if exists
  ret = sprintf(file, "data%sphaseTemp%s", channel, ctime(filet));
  if (ret < 0)
    goto fail;

  // Opens file
  f = fopen(file, "w");
  if (f == NULL)
  {
    perror("failed to open file");
    fclose(f);
    goto fail;
  }
  // Writes data to file, character at a time
  for (i = 0; i < (50 * points); i++)
  {
    ret = fprintf(f, "%c", dat[i]);
    if (ret < 0)
    {
      fclose(f);
      goto fail;
    }
  }

  fclose(f);

  (*fc)++;

  free(file);
  free(channel_com);
  free(point_com);
  free(freq_com);
  free(buff);
  return 0;

fail:
  free(file);
  free(channel_com);
  free(point_com);
  free(freq_com);
  free(buff);
  return -1;
}