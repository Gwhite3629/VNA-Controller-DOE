#ifndef _FILE_H_
#define _FILE_H_

#include <stdio.h>
#include <windows.h>

int find_prof(FILE *fd, int profile, fpos_t *prof_start, fpos_t *prof_end);

int catFile (time_t filet, char *channel, int fstar, int fstop, int points);

void linspace(float min, float size, float max, double *arr);

#endif