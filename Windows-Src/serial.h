#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdint.h>
#include <stddef.h>
#include <windows.h>
#include "GPIB_prof.h"

void print_error(char *context);
HANDLE open_port (const char *dev, uint32_t baud);
int read_port (HANDLE port, char *buf, size_t size);
int write_port (HANDLE port, const char *buf, size_t size);
int GPIB_conf (HANDLE port, int profile);

#endif
