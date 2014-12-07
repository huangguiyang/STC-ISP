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

#include <stdio.h>
#include <assert.h>
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

	#define MAX_FILE_SIZE	(61*1024)

	typedef struct {
		char model[32];
		unsigned short version;
		double frequency;
		int maxbaudrate;
		unsigned char clockmode:1;
		unsigned char clockgain:1;
		unsigned char watchdog:1;
		unsigned char accessram:1;
		unsigned char ale:1;
		unsigned char p01:1;
		unsigned char noteraserom:1;
	} MCUINFO;

	typedef struct {
		int initbaudrate;
		int specbaudrate;
		unsigned char *buf;
		int len;
		int downtype;	// 0-offline, 1-online
	} DOWNLOAD;

	typedef struct {
		const char *port;
		const char *file;
		unsigned char fill;
		DOWNLOAD	download;
	} CONFIG;

	// isp
	int isp_main(int argc, char *argv[]);
	MCUINFO *get_mcu_info();

	// platform
	void printerr(const char *message);
	void sleep_ms(int ms);

#ifdef __cplusplus
}
#endif

#include "ttys.h"
#include "protocol.h"
#include "code.h"
