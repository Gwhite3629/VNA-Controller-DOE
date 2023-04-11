#ifndef _COMMANDS_H_
#define _COMMANDS_H_
#include <windows.h>

int get_DATA(HANDLE fd, char *dat, int points, char *channel, int fstar, int fstop, int *fc, int cal, time_t *filet);

#endif