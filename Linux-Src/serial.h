#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdint.h>
#include <stddef.h>

int open_port (const char *dev, uint32_t baud);
int read_port (int fd, void *buf, size_t size);
int write_port (int fd, const char *buf, size_t size);

#endif
