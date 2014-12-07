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

static int scan_type(unsigned char *buf, int len)
{

	return 0;
}

int process_input(unsigned char *buf, int len, unsigned char fill, unsigned char **obuf, int *olen)
{
	int packs = (len + 511) / 512;
	int bytes = packs * 512;
	if (bytes > MAX_FILE_SIZE) {
		fprintf(stderr, "file too long.\n");
		return -1;
	}

	assert(fill == 0 || fill == 0xFF);

	memset(buf+len, fill, bytes-len);

	if (obuf) *obuf = buf;
	if (olen) *olen = bytes;

	return 0;
}

int hex2bin(unsigned char *hbuf, int hlen, unsigned char **bbuf, int *blen)
{

	return 0;
}

#ifdef __cplusplus
}
#endif
