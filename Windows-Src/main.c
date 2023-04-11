#include "selector.h"
#include "serial.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>

int main(void)
{
	int fd;
	int ret;
	char *dev;
	int baud;

	dev = malloc(64);
	if (dev == NULL) {
		perror("memory error");
		return -1;
	}

	printf("Enter device name: ");
	scanf("%s", dev);
	printf("Enter baud rate: ");
	scanf("%d", &baud);
// Opens serial port
	fd = open_port(dev, baud);
	if (fd < 0)
		return -1;
// Writes GPIB commands
	ret = GPIB_sel(fd, 0);
	if (ret < 0)
		return -1;
// Entire runtime environment contained here
	ret = DATA_sel(fd);
	if (ret < 0)
		return -1;

	close(fd);

	free(dev);

	return 0;
}

