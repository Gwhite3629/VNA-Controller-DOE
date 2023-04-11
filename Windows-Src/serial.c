#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <windows.h>
#include "GPIB_prof.h"
// Error message function
void print_error(const char * context)
{
  DWORD error_code = GetLastError();
  char buffer[256];
  DWORD size = FormatMessageA(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
    NULL, error_code, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
    buffer, sizeof(buffer), NULL);
  if (size == 0) { buffer[0] = 0; }
  fprintf(stderr, "%s: %s\n", context, buffer);
}
// Open serial port
HANDLE open_port (const char * dev, uint32_t baud)
{
// Create device file
    HANDLE port = CreateFileA(dev, GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (port == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    BOOL success = FlushFileBuffers(port);
    if (!success)
    {
        print_error("Failed to flush serial port");
        CloseHandle(port);
        return INVALID_HANDLE_VALUE;
    }
// Set all properties
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 0;
    timeouts.ReadTotalTimeoutConstant = 10000;
    timeouts.ReadTotalTimeoutMultiplier = 1000;
    timeouts.WriteTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    success = SetCommTimeouts(port, &timeouts);
    if (!success)
    {
        print_error("Failed to set serial timeouts");
        CloseHandle(port);
        return INVALID_HANDLE_VALUE;
    }

    DCB state = {0};

    if (!GetCommState(port, &state))
        return INVALID_HANDLE_VALUE;

    state.DCBlength = sizeof(DCB);
    state.BaudRate = CBR_9600;
    state.ByteSize = 8;
    state.fBinary = TRUE;
    state.fErrorChar = FALSE;
    state.fNull = FALSE;
    state.fAbortOnError = FALSE;

    state.fParity = FALSE;
    state.Parity = NOPARITY;
    state.StopBits = ONESTOPBIT;

    state.fRtsControl = RTS_CONTROL_DISABLE;
    state.fOutxCtsFlow = FALSE;
    state.fDtrControl = DTR_CONTROL_DISABLE;
    state.fOutxDsrFlow = FALSE;
    state.fDsrSensitivity = FALSE;
    state.fInX = FALSE;
    state.fOutX = FALSE;

    success = SetCommState(port, &state);
    if (!success)
    {
        print_error("Failed to set serial settings");
        CloseHandle(port);
        return INVALID_HANDLE_VALUE;
    }

    return port;

}
// Write to device file
int write_port(HANDLE port, char *buff, size_t size)
{
    DWORD written;
    BOOL success = WriteFile(port, buff, size, &written, NULL);
    if (!success)
    {
        print_error("Failed to write to port");
        return -1;
    }
    if (written != size)
    {
        print_error("Failed to write all bytes to port");
        return -1;
    }
    usleep(size*200);
    return 0;
}
// Read from device file
SSIZE_T read_port(HANDLE port, char *buff, size_t size)
{
    DWORD received;
    BOOL success = ReadFile(port, buff, size, &received, NULL);
    if (!success)
    {
        print_error("Failed to read from port");
        return -1;
    }
    return received;
}
int GPIB_conf(HANDLE fd, int profile)
{
	int ret;

	switch (profile)
	{
	case 0:
		ret = def(fd);
		if (ret < 0)
			goto fail;
	}
	return 0;

fail:
	return -1;
}