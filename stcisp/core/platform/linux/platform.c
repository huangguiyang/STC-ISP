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

void printerr(const char *message)
{
	perror(message);
}

void sleep_ms(int ms)
{
	struct timespec tv;

	tv.tc_sec = ms / 1000;
	tv.tc_nsec = (ms % 1000) * 1000 * 1000;

	nanosleep(&tv, NULL);
}

int read_file(const char *path, unsigned char *buf, int buflen, int *len)
{
	FILE *fp; 
	int size;
	int bread;
	int p = 0;

	if (buf == NULL || buflen <= 0) {
		fprintf(stderr, "buf invalid\n");
		return -1;
	}
	fp = fopen(path, "r");
	if (fp == NULL) {
		perror("Can't open file");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	if (size > MAX_FILE_SIZE) {
		fprintf(stderr, "file size too long\n");
		fclose(fp);
		return -2;
	}
	if (size > buflen) {
		fprintf(stderr, "buf size too short\n");
		fclose(fp);
		return -2;
	}
	if (len) {
		*len = size;
	}
	fseek(fp, 0, SEEK_SET);
	while (size > 0) {
		bread = fread(buf+p, 1, size, fp);
		if (bread > 0) {
			size -= bread;
			p += bread;
		}
		else {
			break;
		}
	}
	if (size > 0) {
		fprintf(stderr, "read error\n");
		fclose(fp);
		return -3;
	}
	fclose(fp);
	return 0;
}

#ifdef __cplusplus
}
#endif
