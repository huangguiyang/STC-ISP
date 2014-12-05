/*

Copyright (C) 2014 Mario Huang

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT 
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "isp.h"

#ifdef __cplusplus
extern "C" {
#endif

void purge_ttys(pttys fd)
{
	PurgeComm(fd, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
}

int set_baudrate(pttys fd, int baudrate)
{
	termios tms;

	if (!GetCommState(fd, &tms)) {
		printerr("GetCommState error");
		return -1;
	}

	tms.DCBlength = sizeof tms;
	tms.BaudRate = baudrate;
	tms.fRtsControl = RTS_CONTROL_ENABLE;
	tms.fDtrControl = DTR_CONTROL_ENABLE;
	tms.ByteSize = 8;
	tms.Parity = NOPARITY;
	tms.StopBits = ONESTOPBIT;
	tms.EvtChar = 0;

	if (!SetCommState(fd, &tms)) {
		printerr("SetCommState error");
		return -1;
	}

	purge_ttys(fd);

	printf("Changed baudrate to: %d\n", baudrate);

	return 0;
}

pttys open_ttys(const char *port)
{
	pttys fd;
	COMMTIMEOUTS tmos;

	fd = CreateFile(port, 
					GENERIC_READ|GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL/*|FILE_FLAG_OVERLAPPED*/,
					NULL);

	if (fd == NULL_TTYS) {
		printerr("Can't open");
		return NULL_TTYS;
	}
	
	if (!SetupComm(fd, 8192, 8192)) {
		printerr("SetupComm error");
		close_ttys(fd);
		return NULL_TTYS;
	}
	
	tmos.ReadIntervalTimeout = 0;
	tmos.ReadTotalTimeoutMultiplier = 10;
	tmos.ReadTotalTimeoutConstant = 0;
	tmos.WriteTotalTimeoutMultiplier = 0;
	tmos.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(fd,&tmos);

	printf("Open device OK.\n");

	return fd;
}

void close_ttys(pttys fd)
{
	if (fd != NULL_TTYS) {
		CloseHandle(fd);
	}
}

int write_ttys(pttys fd, unsigned char *buf, int len)
{
	DWORD bwrite;
	int ret;
	int p = 0;

	while (len > 0) {
		ret = WriteFile(fd, buf+p, len, &bwrite, 0);
		if (ret) {
			// succeed
			p += bwrite;
			len -= bwrite;
		}
	}

	return 0;
}

int read_ttys(pttys fd, unsigned char *buf, int len)
{
	DWORD bread;
	int ret;
	int p = 0;

	while (len > 0) {
		ret = ReadFile(fd, buf+p, len, &bread, 0);
		if (ret) {
			// succeed
			p += bread;
			len -= bread;
		}
	}

	return 0;
}

void keep_send(pttys fd, unsigned char *ibuf, int len)
{
	unsigned char buf[] = {0x7F};
	DWORD bwrite;
	DWORD bread;

	ClearCommError(fd, 0, 0);

	while (1) {
		
		if (WriteFile(fd, buf, 1, &bwrite, 0) && bwrite == 1) {
			// read 
			if (ReadFile(fd, ibuf, 1, &bread, 0) && bread > 0) {
				break;
			}
		}
		else {
			printerr("write error\n");
		}
	}
}

#ifdef __cplusplus
}
#endif