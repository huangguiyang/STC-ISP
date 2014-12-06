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

static CONFIG usrConfig;

static int isp(CONFIG *config)
{
	pttys fd;
	const char *port = config->port;
	int initbaudrate = config->download.initbaudrate;
	int specbaudrate = config->download.specbaudrate;

	printf("Hello, %s: %d\n", port, specbaudrate);

	fd = open_ttys(port);
	if (fd == NULL_TTYS) {
		return -1;
	}

	set_baudrate(fd, initbaudrate);
	
	if (download(fd, &config->download) < 0) {
		close_ttys(fd);
		return -1;
	}

	close_ttys(fd);

	printf("Burned! Success!\n");

	return 0;
}

MCUINFO *get_mcu_info()
{
	static MCUINFO mcuInfo;

	return &mcuInfo;
}

int isp_main(int argc, char *argv[])
{
	int ret = 0;
	int len;
	unsigned char *buf = (unsigned char *) malloc(MAX_FILE_SIZE);

	//NOTE: hard code now, need to be updated...
	usrConfig.port = "COM4";
	usrConfig.file = "D:\\µ¥Æ¬»ú\\led.bin";
	usrConfig.fill = 0xFF;
	usrConfig.download.initbaudrate = 1200;
	usrConfig.download.specbaudrate = 9600;
	usrConfig.download.buf = NULL;
	usrConfig.download.downtype = 0;

	if (read_file(usrConfig.file, buf, MAX_FILE_SIZE, &len) < 0) {
		ret = -1;
		goto clean;
	}

	printf("read file ok: %d bytes.\n", len);

	if (process_input(buf, len, usrConfig.fill, &usrConfig.download.buf, &usrConfig.download.len) < 0) {
		ret = -1;
		goto clean;
	}

	if (isp(&usrConfig) < 0) {
		fprintf(stderr, "failed.\n");
	}

clean:
	free(buf);
	if (usrConfig.download.buf && buf != usrConfig.download.buf) 
		free(usrConfig.download.buf);
	return ret;
}

#ifdef __cplusplus
}
#endif
