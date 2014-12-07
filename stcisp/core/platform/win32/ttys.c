/*

Copyright (C) 2014 Mario Huang

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "isp.h"

#ifdef __cplusplus
extern "C" {
#endif

	static int mode;

void purge_ttys(pttys fd)
{
	PurgeComm(fd, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
}

static void set_rts(pttys fd)
{
	EscapeCommFunction(fd, SETRTS);
}

static void clr_rts(pttys fd)
{
	EscapeCommFunction(fd, CLRRTS);
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
	tms.DCBlength = sizeof tms;
	tms.fRtsControl = mode == 0 ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE;
	tms.fDtrControl = mode == 0 ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE;
	tms.ByteSize = 8;
	tms.Parity = NOPARITY;
	tms.StopBits = ONESTOPBIT;
	tms.EvtChar = mode == 0 ? 0 : 0x68;

	if (!SetCommState(fd, &tms)) {
		printerr("SetCommState error");
		return -1;
	}

	purge_ttys(fd);

	printf("Changed baudrate to: %d\n", baudrate);

	return 0;
}

pttys open_ttys(const char *port, int flags)
{
	pttys fd;
	COMMTIMEOUTS tmos;

	mode = flags & TTY_FLAG_ONLINE;
	fd = CreateFile(port, 
					GENERIC_READ|GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
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

	if (mode == 0) {
		tmos.ReadIntervalTimeout = 0;
		tmos.ReadTotalTimeoutMultiplier = 10;
		tmos.ReadTotalTimeoutConstant = 0;
		tmos.WriteTotalTimeoutMultiplier = 0;
		tmos.WriteTotalTimeoutConstant = 5000;
		SetCommTimeouts(fd,&tmos);
	}
	else {
		tmos.ReadIntervalTimeout = 0;
		tmos.ReadTotalTimeoutMultiplier = 0;
		tmos.ReadTotalTimeoutConstant = 0;
		tmos.WriteTotalTimeoutMultiplier = 0;
		tmos.WriteTotalTimeoutConstant = 0;
		SetCommTimeouts(fd,&tmos);
	}

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

struct thcontext {
	pttys	fd;
};

DWORD WINAPI wait_thread(LPVOID param)
{
	DWORD evmask;
	struct thcontext *pcontext = (struct thcontext *) param;
	pttys fd = pcontext->fd;
	UINT ret;

	SetCommMask(fd, EV_RXFLAG);

	while (1) {
		if (WaitCommEvent(fd, &evmask, NULL)) {
			if (evmask & EV_RXFLAG) {
				// OK
				ret = 0;
				break;
			}
			// continue
		}
		else {
			DWORD err = GetLastError();
			if (err == ERROR_IO_PENDING) {
				// continue
			}
			else {
				fprintf(stderr, "WaitCommEvent error: %d\n", err);
				ret = -1;
				break;
			}
		}
	}

	return ret;
}

static void CALLBACK sfunc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	DWORD *set = (DWORD *) dwUser;
	printf("sfunc: %d\n", *set);
	*set = ! (*set);
}

static void CALLBACK wfunc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	unsigned char buf[] = {0x7F, 0x7F};
	DWORD *pfd = (DWORD *) dwUser;
	pttys fd = (pttys) (*pfd);

	if (WriteFile(fd, buf, 2, NULL, 0)) {

	}
	else {
		fprintf(stderr, "WriteFile error: %d\n", GetLastError());
	}
}

int keep_interupt(pttys fd)
{
	int ret;
	DWORD set = 1;
	struct thcontext param = {fd, };
	HANDLE thread;
	DWORD exitcode;
	long seticks;
	long wticks;

	thread = CreateThread(NULL, 0, wait_thread, (LPVOID)&param, 0, NULL);
	if (thread == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Can't create thread: %d\n", GetLastError());
		return -1;
	}
	sleep_ms(20);

	seticks = GetTickCount();
	wticks = GetTickCount();


	// wait on mask
	while (1) {
		long cwticks = GetTickCount();
		long cseticks = GetTickCount();
		if (cseticks - seticks >= 600) {
			set = !set;
			seticks = cseticks;
		}
		
		if (GetExitCodeThread(thread, &exitcode)) {
			if (exitcode != STILL_ACTIVE) {
				ret = exitcode;
				break;
			}
			// continue
		}
		else {
			fprintf(stderr, "Can't get thread exit code: %d\n", GetLastError());
			ret = -1;
			break;
		}

		// set RTS (600ms)
		// 20ms write 2 bytes FF
		// clear RTS (600ms)
		if (set) {
			set_rts(fd);
		}
		else {
			clr_rts(fd);
		}
		
		if (cwticks - wticks > 20) {
			unsigned char buf[] = {0x7F, 0x7F};

			if (WriteFile(fd, buf, 2, NULL, 0)) {
				fprintf(stderr, "WriteFile \n");
			}
			else {
				fprintf(stderr, "WriteFile error: %d\n", GetLastError());
			}
			wticks = cwticks;
		}
	}


	if (ret == 0)
		printf("OK. Got the response.\n");
	else 
		printf("No. Get errors.\n");

	CloseHandle(thread);

	return ret;
}

#ifdef __cplusplus
}
#endif