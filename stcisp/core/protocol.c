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

#define END_BYTE	0x16

// commands
#define CMD_CHECK_BAUD		0x8F
#define CMD_CONFIRM_BAUD	0x8E
#define CMD_CLOSE			0x82
#define CMD_ACK				0x80
#define CMD_ERASE			0x84
#define CMD_DATA			0x00
#define CMD_SET_OPTIONS		0x8D
#define CMD_0X50			0x50	// unknown commond

#define RES_ACK				0x80
#define RES_0X10			0x10

static unsigned char COMMON_HEAD[] = {0x46, 0xB9};
static unsigned char RECV_ID[] = {0x68, 0x00};
static unsigned char SEND_ID[] = {0x6A, 0x00};

#define HEAD_SIZE	sizeof COMMON_HEAD
#define TAIL_SIZE	2
#define RECOG_SIZE	(HEAD_SIZE + sizeof RECV_ID + 1)

#define CODE_DATA_SIZE		0x80

#define cmd(buf)	(buf[RECOG_SIZE])

// MCU Options
#define OPTION_CLOCK_SPEED		0001
#define OPTION_P01				0004
#define OPTION_ERASE_EEPROM		0010
#define	OPTION_CLOCK_GAIN		0020
#define OPTION_ALE				0040
#define OPTION_ACCESS_RAM		0100
#define OPTION_WATCH_DOG		0200

// baud packets
#define BAUD_PACKET_SIZE	6
static unsigned char B1200_PACKET[] = {0xFE, 0xC9, 0x01, 0x6E, 0x28, 0x82};
static unsigned char B2400_PACKET[] = {0xFF, 0x64, 0x00, 0xFE, 0x28, 0x82};
static unsigned char B4800_PACKET[] = {0xFF, 0xB2, 0x00, 0x9C, 0x28, 0x82};
static unsigned char B9600_PACKET[] = {0xFF, 0xD9, 0x00, 0x4E, 0x28, 0x82};
static unsigned char B19200_PACKET[] = {0xFF, 0xED, 0x00, 0x26, 0x28, 0x82};
static unsigned char B28800_PACKET[] = {0xFF, 0xF3, 0x00, 0x1A, 0x28, 0x82};

static void printb(const char *lead, unsigned char *buf, int len)
{
	int i;
	if (lead){
		printf("%s", lead);
	}
	for (i=0; i < len; i++) {
		printf("%02X ", buf[i]);
	}
	printf("\n");
}

static unsigned char *baudrate_packet(int baudrate)
{
	switch (baudrate) {
		case 1200: return B1200_PACKET;
		case 2400: return B2400_PACKET;
		case 4800: return B4800_PACKET;
		case 9600: return B9600_PACKET;
		case 19200: return B19200_PACKET;
		case 28800: 
		case 115200:
			return B28800_PACKET;
		default:
			fprintf(stderr, "Unsupport baudrate %d\n", baudrate);
			return NULL;
	}
}

static unsigned char checksum(unsigned char *buf, int len)
{
	unsigned int cksum = 0;
	
	while (len > 0) {
		cksum += *buf++;
		len--;
	}
	
	return cksum & 0xFF;
}

static void gen_cmd_buf(int cmd, unsigned char *data, int datalen, unsigned char *buf, int *buflen)
{
	int len = sizeof COMMON_HEAD + sizeof SEND_ID + 1 + 1 + datalen + TAIL_SIZE;
	unsigned char *packet = buf;
	int p = 0;
	
	assert(buf && buflen);
	
	memcpy(packet, COMMON_HEAD, sizeof COMMON_HEAD);
	p += sizeof COMMON_HEAD;
	memcpy(packet+p, SEND_ID, sizeof SEND_ID);
	p += sizeof SEND_ID;
	packet[p++] = len - sizeof COMMON_HEAD;
	packet[p++] = (unsigned char) cmd;
	if (data) {
		memcpy(packet+p, data, datalen);
		p += datalen;
	}
	// checksum
	packet[p] = checksum(packet+HEAD_SIZE, p-HEAD_SIZE);
	packet[++p] = END_BYTE;
	assert(++p == len);
	
	*buflen = len;
}

static int verify_checksum(unsigned char *buf, int len)
{
	unsigned char cksum;

	cksum = checksum(buf+HEAD_SIZE, len-TAIL_SIZE-HEAD_SIZE);

	if (cksum != buf[len-TAIL_SIZE]) {
		fprintf(stderr, "invalid checkusm %02X, expected %02X.\n", buf[len-TAIL_SIZE], cksum);
		return -1;
	}

	return 0;
}

static int read_packet(pttys fd, unsigned char *buf, int buflen)
{
	unsigned char head[RECOG_SIZE];
	unsigned char *packet = buf;
	int packlen;

	assert(buf && buflen);

	read_ttys(fd, head, RECOG_SIZE);

	// verify
	if (head[0] != COMMON_HEAD[0] || 
		head[1] != COMMON_HEAD[1] ||
		head[2] != RECV_ID[0] || 
		head[3] != RECV_ID[1]) {
		fprintf(stderr, "invalid packet header: %02X %02X %02X %02X\n", 
			head[0], head[1], head[2], head[3]);
		return -1;
	}

	if (head[4] < 5) {
		fprintf(stderr, "invalid packet length: %d", head[4]);
		return -1;
	}

	packlen = head[4] + HEAD_SIZE;
	if (packlen > buflen) {
		fprintf(stderr, "input buffer too small: %d, expected %d", buflen, packlen);
		return -1;
	}

	memcpy(packet, head, RECOG_SIZE);
	read_ttys(fd, packet+RECOG_SIZE, head[4]-3);

	// verify checksum
	if (verify_checksum(packet, packlen) < 0) {
		return -2;
	}
	
	printb("Recv: ", buf, packlen);

	return packlen;
}

static int write_packet(pttys fd, unsigned char *buf, int len)
{
	write_ttys(fd, buf, len);
	printb("Send: ", buf, len);

	return len;
}

static unsigned char get_options()
{
	unsigned char options = 0;
	MCUINFO *mcuinfo = get_mcu_info();

	if (mcuinfo->clockmode)
		options |= OPTION_CLOCK_SPEED;
	if (mcuinfo->clockgain)
		options |= OPTION_CLOCK_GAIN;
	if (mcuinfo->watchdog)
		options |= OPTION_WATCH_DOG;
	if (mcuinfo->accessram)
		options |= OPTION_ACCESS_RAM;
	if (mcuinfo->ale)
		options |= OPTION_ALE;
	if (mcuinfo->noteraserom)
		options |= OPTION_ERASE_EEPROM;
	if (mcuinfo->p01)
		options |= OPTION_P01;

	return options;
}

static void parse_options(MCUINFO *mcuinfo, unsigned char options)
{
#define BOOLEAN(val)			val ? 1 : 0

	mcuinfo->clockmode = BOOLEAN(options & OPTION_CLOCK_SPEED);
	mcuinfo->clockgain = BOOLEAN(options & OPTION_CLOCK_GAIN);
	mcuinfo->watchdog = BOOLEAN(options & OPTION_WATCH_DOG);
	mcuinfo->accessram = BOOLEAN(options & OPTION_ACCESS_RAM);
	mcuinfo->ale = BOOLEAN(options & OPTION_ALE);
	mcuinfo->noteraserom = BOOLEAN(options & OPTION_ERASE_EEPROM);
	mcuinfo->p01 = BOOLEAN(options & OPTION_P01);
}

static void print_options(MCUINFO *mcuinfo)
{
	if (mcuinfo->clockmode) {
		printf("系统频率为12T（单倍速）模式\n");
	}
	else {
		printf("系统频率为6T（双倍速）模式\n");
	}

	if (mcuinfo->clockgain) {
		printf("振荡器的放大增益不降低\n");
	}
	else {
		printf("振荡器的放大增益降低\n");
	}

	if (mcuinfo->watchdog) {
		printf("当看门狗启动后，任何复位都可以停止看门狗\n");
	}
	else {
		printf("当看门狗启动后，只有断电可以停止看门狗\n");
	}

	if (mcuinfo->accessram) {
		printf("MCU内部的扩展RAM可用\n");
	}
	else {
		printf("MCU内部的扩展RAM不可用\n");
	}

	if (mcuinfo->ale) {
		printf("ALE脚的功能选择仍然为ALE功能脚\n");
	}
	else {
		printf("ALE脚的功能选择为P4功能脚\n");
	}

	if (mcuinfo->p01) {
		printf("P1.0和P1.1与下次下载无关\n");
	}
	else {
		printf("为0/0才可下载用户程序\n");
	}

	if (mcuinfo->noteraserom) {
		printf("下次下载用户程序时，不擦除用户EEPROM区\n");
	}
	else {
		printf("下次下载用户程序时，擦除用户EEPROM区\n");
	}
}

static void parse_mcu_info(unsigned char *buf, int len)
{
	MCUINFO *mcuinfo = get_mcu_info();
	int i;
	int pulse_width_sum = 0;
	int pulse_width;

	// model: maybe buf[23-25], hard code here
	strncpy(mcuinfo->model, "STC90C516RD+", sizeof "STC90C516RD+");
	
	// version
	mcuinfo->version = buf[20] << 8 + buf[21];

	// frequency
	for (i=4; i < 20; i+=2) {
		pulse_width_sum += buf[i] << 8 + buf[i+1];
	}
	pulse_width = pulse_width_sum / 8;
	mcuinfo->frequency = pulse_width * 1200 * 12 / 6.99786;

	// options
	parse_options(mcuinfo, buf[22]);

	printf("\n");
	printf("型号: %s\n", mcuinfo->model);
	printf("固件版本: %d.%d%C\n", 
		(mcuinfo->version >> 12) & 0x0F,
		(mcuinfo->version >> 8) & 0x0F,
		mcuinfo->version & 0x0FF);
	printf("时钟频率: %.3f MHz\n", mcuinfo->frequency / 1000000);
	print_options(mcuinfo);
	printf("\n");
}

int handshake(pttys fd)
{
	unsigned char ibuf[128];
	int p;

	printf("Handshaking...\n");

	keep_send(fd, ibuf, sizeof ibuf);

	read_ttys(fd, &ibuf[1], 2);

	// verify
	if (ibuf[0] != RECV_ID[0] || ibuf[1] != RECV_ID[1]) {
		fprintf(stderr, "invalid packet header: %02X %02X\n", ibuf[0], ibuf[1]);
		return -1;
	}
	if (ibuf[2] < 5) {
		fprintf(stderr, "invalid packet length: %d", ibuf[2]);
		return -1;
	}

	read_ttys(fd, &ibuf[3], ibuf[2]-3);

	// verify
	p = ibuf[2];
	if (ibuf[p-1] != END_BYTE) {
		fprintf(stderr, "invalid end byte: %02X", ibuf[p-1]);
		return -1;
	}

	// ok
	printf("Handshake OK.\n");
	printb(NULL, ibuf, p);
	parse_mcu_info(ibuf, p);

	return 0;
}

static int check_baudrate(pttys fd, int baudrate)
{
	unsigned char *data;
	int datalen = BAUD_PACKET_SIZE;
	unsigned char buf[256];
	int len;
	unsigned char ibuf[256];
	int ilen;
	int i;

	data = baudrate_packet(baudrate);
	if (data == NULL) {
		return -1;
	}
	gen_cmd_buf(CMD_CHECK_BAUD, data, datalen, buf, &len);

	write_packet(fd, buf, len);
	sleep_ms(200);

	// change baudrate
	set_baudrate(fd, baudrate);

	if ((ilen = read_packet(fd, ibuf, sizeof ibuf)) < 0) {
		return -1;
	}

	// verify data
	if (datalen != ilen-RECOG_SIZE-1-TAIL_SIZE) {
		fprintf(stderr, "datalen not equal\n");
		return -2;
	}
	for (i=RECOG_SIZE+1; i < ilen-TAIL_SIZE; i++) {
		if (buf[i] != ibuf[i]) {
			fprintf(stderr, "data not equal\n");
			return -2;
		}
	}

	printf("Baudrate is OK.\n");

	return 0;
}

static int confirm_baudrate(pttys fd, int initbaudrate, int specbaudrate)
{
	unsigned char *data;
	int datalen = BAUD_PACKET_SIZE-1;
	unsigned char buf[256];
	int len;
	unsigned char ibuf[256];

	data = baudrate_packet(specbaudrate);
	if (data == NULL) {
		return -1;
	}
	gen_cmd_buf(CMD_CONFIRM_BAUD, data, datalen, buf, &len);

	set_baudrate(fd, initbaudrate);

	write_packet(fd, buf, len);
	sleep_ms(200);

	set_baudrate(fd, specbaudrate);

	if (read_packet(fd, ibuf, sizeof ibuf) < 0) {
		return -1;
	}

	return 0;
}

int verify_baudrate(pttys fd, int initbaudrate, int specbaudrate)
{
	if (check_baudrate(fd, specbaudrate) < 0) {
		close_ttys(fd);
		return -1;
	}

	if (confirm_baudrate(fd, initbaudrate, specbaudrate) < 0) {
		close_ttys(fd);
		return -1;
	}

	return 0;
}

int rehandshake(pttys fd)
{
	// send rehandshake packet
	unsigned char data[] = {0x00, 0x00, 0x36, 0x01, 0xF1, 0x30};
	unsigned char buf[32];
	int len;
	unsigned char ibuf[32];
	int times = 5;

	gen_cmd_buf(CMD_ACK, data, sizeof data, buf, &len);

	printf("Re-Handshaking...\n");

	while (times-- > 0) {
		write_packet(fd, buf, len);
		
		if (read_packet(fd, ibuf, sizeof ibuf) < 0 ||
			cmd(ibuf) != RES_ACK) {
			break;
		}
	}

	if (times > 0) {
		fprintf(stderr, "invalid response: %02X\n", cmd(ibuf));
		return -1;
	}

	printf("Re-Handshaking OK.\n");

	return 0;
}

int download(pttys fd, unsigned char *buf, int len)
{
	// send prepare to download packet
	unsigned char data[] = {0x01, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33};
	unsigned char obuf[32];
	int olen;
	unsigned char ibuf[32];
	unsigned short p = 0;
	unsigned char cbuf[160];
	int clen;
	unsigned char cdata[160];

	assert(buf && len <= MAX_FILE_SIZE && len % CODE_DATA_SIZE == 0);

	printf("Erasing rom...\n");

	gen_cmd_buf(CMD_ERASE, data, sizeof data, obuf, &olen);
	write_packet(fd, obuf, olen);
	
	if (read_packet(fd, ibuf, sizeof ibuf) < 0 ||
		cmd(ibuf) != RES_ACK) {
		fprintf(stderr, "invalid response: %02X\n", cmd(ibuf));
		return -1;
	}

	printf("Downloading...\n");

	// place hodler
	cdata[0] = cdata[1] = 0;
	// size
	cdata[4] = 0;
	cdata[5] = CODE_DATA_SIZE;
	// Send data
	while (p < len) {

		// address
		cdata[2] = (p >> 8) & 0xFF;
		cdata[3] = p & 0xFF;

		memcpy(&cdata[6], buf+p, CODE_DATA_SIZE);

		gen_cmd_buf(CMD_DATA, cdata, CODE_DATA_SIZE+6, cbuf, &clen);
		write_packet(fd, cbuf, clen);

		if (read_packet(fd, ibuf, sizeof ibuf) < 0 ||
			cmd(ibuf) != RES_ACK) {
			fprintf(stderr, "invalid response: %02X\n", cmd(ibuf));
			return -1;
		}

		p += CODE_DATA_SIZE;
	}

	printf("Download OK.\n");

	return 0;
}

int update_options(pttys fd)
{
	unsigned char data[12] = {0xFD, 0xFF, 0xF6, 0xFF};
	int datalen = 4;
	unsigned char buf[32];
	int len;
	unsigned char ibuf[32];
	int ilen;
	int i;

	data[0] = get_options();
	gen_cmd_buf(CMD_SET_OPTIONS, data, datalen, buf, &len);
	write_packet(fd, buf, len);

	if ((ilen = read_packet(fd, ibuf, sizeof ibuf)) < 0) {
		return -1;
	}
	if (cmd(ibuf) != CMD_SET_OPTIONS) {
		fprintf(stderr, "update options: invalid response %02X\n", cmd(ibuf));
		return -1;
	}

	// verify data
	if (datalen != ilen - RECOG_SIZE - 1 - TAIL_SIZE) {
		fprintf(stderr, "update options: datalen not equal\n");
		return -2;
	}

	for (i=RECOG_SIZE+1; i < ilen-TAIL_SIZE; i++) {
		if (buf[i] != ibuf[i]) {
			fprintf(stderr, "data not equal\n");
			return -2;
		}
	}

	printf("Update options OK.\n");
	print_options(get_mcu_info());

	return 0;
}

void goodbye(pttys fd)
{
	unsigned char buf[32];
	int len;
	unsigned char ibuf[32];

	// 50 command
	gen_cmd_buf(CMD_0X50, NULL, 0, buf, &len);
	write_packet(fd, buf, len);

	if (read_packet(fd, ibuf, sizeof ibuf) < 0 ||
		cmd(ibuf) != RES_0X10) {
		fprintf(stderr, "invalid response %02X\n", cmd(ibuf));
	}

	// close command
	gen_cmd_buf(CMD_CLOSE, NULL, 0, buf, &len);
	write_packet(fd, buf, len);
	sleep_ms(50);
}

#ifdef __cplusplus
}
#endif