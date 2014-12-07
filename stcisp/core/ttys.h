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

#ifdef __cplusplus
extern "C" {
#endif

#define TTY_FLAG_ONLINE		0001

	// ttys
	pttys open_ttys(const char *port, int flags);
	void close_ttys(pttys fd);
	int set_baudrate(pttys fd, int baudrate);
	int write_ttys(pttys fd, unsigned char *buf, int len);
	int read_ttys(pttys fd, unsigned char *buf, int len);
	void keep_send(pttys fd, unsigned char *ibuf, int len);
	int keep_interupt(pttys fd);

#ifdef __cplusplus
}
#endif