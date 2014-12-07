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

void purge_ttys(pttys fd)
{
	tcflush(fd, TCIOFLUSH);
}

int set_baudrate(pttys fd, int baudrate)
{
	

	printf("Changed baudrate to: %d\n", baudrate);

	return 0;
}

pttys open_ttys(const char *port)
{
	pttys fd;

	printf("Open device OK.\n");

	return fd;
}

void close_ttys(pttys fd)
{
	if (fd != NULL_TTYS) {
		close(fd);
	}
}

int write_ttys(pttys fd, unsigned char *buf, int len)
{
	

	return 0;
}

int read_ttys(pttys fd, unsigned char *buf, int len)
{

	return 0;
}

void keep_send(pttys fd, unsigned char *ibuf, int len)
{
	fd_set rfds, wfds;

	while (1) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		FD_ZERO();
	}
	
}

#ifdef __cplusplus
}
#endif
