#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "file.h"
// Gets position in file of current selected profile
int find_prof(FILE *fd, int profile, fpos_t *prof_start, fpos_t *prof_end) {
  int found = 0;
  char *buffer = NULL;
  char *check = NULL;
// Allocate arrays and memset to 0
  buffer = malloc(8);
  if (buffer == NULL) {
    perror("memory error");
    goto fail;
  }
  memset(buffer, 0, 8);
  check = malloc(8);
  if (check == NULL) {
    perror("memory error");
    goto fail;
  }
  memset(check, 0, 8);
  sprintf(check, "#%d", profile);
// Get start and end positions of profile
  do {
    buffer[1] = fgetc(fd);
    if (strcmp(buffer, check) == 0 && found == 0) {
      fgetpos(fd, prof_start);
      found = 1;
    } else if (strcmp(buffer, check) == 0 && found == 1) {
      fgetpos(fd, prof_end);
      found = 2;
    } else {
      buffer[0] = buffer[1];
    }
  } while (found < 2);

  free(buffer);
  free(check);
  return 0;

fail:
  free(buffer);
  free(check);
  return -1;
}

int catFile (time_t filet, char *channel, int fstar, int fstop, int points)
{
  FILE *final;
  FILE *MAG;
  FILE *PHASE;
  fpos_t pos;
  char *finalName = NULL;
  char *MAGName = NULL;
  char *PHASEName = NULL;
  double *freq = NULL;

  int ret = 0;
  int i;
  char *temp = NULL;

  freq = (double *)malloc(points*sizeof(double));
  if (freq == NULL) {
    perror("memory error");
    goto fail;
  }
  memset(freq, 0, points);

  MAGName = malloc(64);
  if (MAGName == NULL) {
    perror("memory error");
    goto fail;
  }
  PHASEName = malloc(64);
  if (PHASEName == NULL) {
    perror("memory error");
    goto fail;
  }
  finalName = malloc(64);
  if (finalName == NULL) {
    perror("memory error");
    goto fail;
  }
  temp = malloc(26);
  if (temp == NULL) {
    perror("memory error");
    goto fail;
  }
  
  linspace(fstar, points, fstop, freq);

  ret = sprintf(MAGName, "data%smagTemp%s", channel, ctime(&filet));
  if (ret < 0)
    goto fail;

  MAG = fopen(MAGName, "r+");
  if (MAG == NULL) {
    perror("failed to open file");
    fclose(MAG);
    goto fail;
  }
  fseek(MAG, 0, SEEK_SET);
  ret = sprintf(PHASEName, "data%sphaseTemp%s", channel, ctime(&filet));
  if (ret < 0)
    goto fail;
  PHASE = fopen(PHASEName, "r+");
  if (PHASE == NULL) {
    perror("failed to open file");
    fclose(PHASE);
    goto fail;
  }
  fseek(PHASE, 0, SEEK_SET);
  ret = sprintf(finalName, "%s%s", channel, ctime(&filet));
  if (ret < 0)
    goto fail;
  final = fopen(finalName, "w");
  if (PHASE == NULL) {
    perror("failed to open file");
    fclose(final);
    goto fail;
  }
  fseek(final, 0, SEEK_SET);
  fprintf(final, "   MAGNITUDE            ,   PHASE                ,  FREQUENCY\n");
  for (i=0;i<points;i++) {
    fgets(temp, 26, MAG);
    fprintf(final, "%s", temp);
    
    fgetpos(MAG, &pos);
    fseek(MAG, 25, SEEK_CUR);
    
    fgets(temp, 26, PHASE);
    fprintf(final, "%s", temp);
    
    fgetpos(PHASE, &pos);
    fseek(PHASE, 25, SEEK_CUR);

    fprintf(final, "  %lf\n", freq[i]);
  }

  fclose(MAG);
  fclose(PHASE);
  ret = remove(MAGName);
  if (ret < 0) {
    perror("failed to remove temp file");
    goto fail;
  }
  ret = remove(PHASEName);
  if (ret < 0) {
    perror("failed to remove temp file");
    goto fail;
  }
  fclose(final);
  free(freq);
  free(MAGName);
  free(PHASEName);
  free(finalName);
  free(temp);
  return 0;

fail:
  fclose(MAG);
  fclose(PHASE);
  fclose(final);
  free(freq);
  free(MAGName);
  free(PHASEName);
  free(finalName);
  free(temp);
  return -1;
}

void linspace(float min, float size, float max, double *arr)
{
  float iter = (max - min) / (size-1);
  for (int i=0; i<size; i++) {
    arr[i] = min + (i*iter);
  }
}