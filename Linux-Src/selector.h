#ifndef _SELECTOR_H_
#define _SELECTOR_H_

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

struct args {
  FILE *profdat;
  fpos_t *prof_start;
  fpos_t *prof_end;
  int fd;
  int profile;
  int points;
  int channum;
  char chan[4];
  char *testtype;
  int interval;
  bool time_flag;
  int testtime;
  int fstar;
  int fstop;
  int fc;
};

int GPIB_sel (int fd, int profile);

int prep_args(struct args **prof, int fd);

int DATA_sel (int fd);

#endif