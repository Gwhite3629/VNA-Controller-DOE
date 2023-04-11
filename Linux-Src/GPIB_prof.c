#include "GPIB_prof.h"
#include "serial.h"

int def(int fd)
{
	int ret;
// Manually writes GPIB control commands
	ret = write_port(fd, "++mode 1\r", 9);
	if (ret < 0)
		goto fail;
	ret = write_port(fd, "++addr 16\r", 10);
	if (ret < 0)
		goto fail;
	ret = write_port(fd, "++eoi 0\r", 8);
	if (ret < 0)
		goto fail;
	ret = write_port(fd, "++eot_enable 1\r", 15);
	if (ret < 0)
		goto fail;
	ret = write_port(fd, "++eot_char 13\r", 14);
	if (ret < 0)
		goto fail;
	ret = write_port(fd, "++ifc\r", 6);
	if (ret < 0)
		goto fail;
	ret = write_port(fd, "++auto 1\r", 9);
	if (ret < 0)
		goto fail;

	return 0;
fail:
	return -1;
}
