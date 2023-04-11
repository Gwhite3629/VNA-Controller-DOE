#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdint.h>
#include <unistd.h>

#include "serial.h"

struct termios options;

// Opens serial device file
int open_port(const char *dev, uint32_t baud)
{
	int fd;
	int ret;
	char *baud_f;
	struct termios opt;

	baud_f = malloc(8 * sizeof(char));
	if (baud_f == NULL)
	{
		perror("memory error");
		goto fail;
	}

	fd = open(dev, O_RDWR, 0777);
	if (fd < 0)
	{
		perror(dev);
		goto fail;
	}
	ret = tcflush(fd, TCIOFLUSH);
	if (ret)
	{
		perror("tcgetattr failed");
		goto fail;
	}
// Flags for correct reading and writing to and from serial device file
	opt.c_cflag = (opt.c_cflag & ~CSIZE) | CS8;
	opt.c_cflag &= ~IGNBRK;
	opt.c_lflag = 0;
	opt.c_oflag = 0;

	opt.c_cc[VTIME] = 5;
	opt.c_cc[VMIN] = 1;

	opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	opt.c_cflag |= (CLOCAL | CREAD);
	opt.c_cflag &= ~(PARENB | PARODD);
	opt.c_cflag |= 0;
	opt.c_cflag &= ~CSTOPB;

	ret = sprintf(baud_f, "B%d", baud);
	if (ret < 0)
	{
		perror("memory error");
		goto fail;
	}
	baud = (uintptr_t)baud_f;
	cfsetospeed(&opt, baud);
	cfsetispeed(&opt, baud);

	ret = tcsetattr(fd, TCSANOW, &opt);
	if (ret)
	{
		perror("tcsetarre failed");
		goto fail;
	}

	free(baud_f);
	return fd;

fail:
	free(baud_f);
	close(fd);
	return -1;
}
// Reat function that times out based on machine timeout
int read_port(int fd, void *buf, const size_t size)
{
	ssize_t r;
	size_t received;
	received = 0;
	while (received < size)
	{
		r = read(fd, buf + received, size - received);
		if (r < 0)
		{
			perror("failed to read from port");
			return -1;
		}
		if (r == 0)
		{
			break;
		}
		received += r;
	}
	return received;
}
// Simple write function that sleeps after it finishes a write
int write_port(int fd, const char *buf, const size_t size)
{
	ssize_t res;
	res = write(fd, buf, size);
	if (res != (ssize_t)size)
	{
		perror("failed to write to port");
		return -1;
	}
	usleep(size * 100);
	return 0;
}
