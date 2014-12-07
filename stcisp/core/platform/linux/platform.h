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

#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef int				pttys;
	#define NULL_TTYS		-1

	// platform
	void printerr(const char *message);
	void sleep_ms(int ms);
	int read_file(const char *path, unsigned char *buf, int buflen, int *len);

#ifdef __cplusplus
}
#endif
