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
