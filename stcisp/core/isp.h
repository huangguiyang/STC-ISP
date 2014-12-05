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
		const char *port;
		const char *file;
		int initbaudrate;
		int specbaudrate;
		unsigned char *buf;
		int len;
		unsigned char fill;
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
