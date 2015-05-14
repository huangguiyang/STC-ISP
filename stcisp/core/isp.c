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

static CONFIG usrConfig;

static int isp(CONFIG *config)
{
	pttys fd;
	const char *port = config->port;
	int initbaudrate = config->download.initbaudrate;
	int specbaudrate = config->download.specbaudrate;
	int mode = config->download.downtype == 0 ? 0 : TTY_FLAG_ONLINE;

	fd = open_ttys(port, mode);
	if (fd == NULL_TTYS) {
		return -1;
	}

	printf("Hello, %s: %d [fd: %d]\n", port, specbaudrate, fd);

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
	usrConfig.file = "led.bin";
	usrConfig.fill = 0xFF;
	usrConfig.download.initbaudrate = 1200;
	usrConfig.download.specbaudrate = 9600;
	usrConfig.download.buf = NULL;
	usrConfig.download.downtype = 1;

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
