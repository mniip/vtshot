#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "reader_generic.h"
#include "vcsa.h"
#include "log.h"

static void read_all(int file_desc, void *buf, size_t size)
{
	while(size)
	{
		int ret;
		if(0 > (ret = read(file_desc, buf, size)))
			die("vcsa_capture: Error while reading from VCSA device at offset 0x%lx: %s\n", lseek(file_desc, 0, SEEK_CUR), strerror(errno));
		if(!ret)
			say("vcsa_capture: Read ended early at offset 0x%lx\n", lseek(file_desc, 0, SEEK_CUR));
		size -= ret;
		buf += ret;
	}
}

descriptor vcsa_init(char const *device)
{
	say("vcsa_init: Opening the TTY device '%s'\n", device);
	int file_desc = open(device, O_WRONLY);
	if(file_desc < 0)
		die("vcsa_init: Could not open TTY device '%s' for writing: %s\n", device, strerror(errno));

	struct stat tty;
	if(0 > fstat(file_desc, &tty))
		die("vcsa_init: Could not stat the TTY device: %s\n", strerror(errno));
	say("vcsa_init: Device id = 0x%04lx\n", tty.st_rdev);
	if((tty.st_rdev & 0xFF00) != 0x0400)
		die("vcsa_init: '%s' is not a TTY device (Expected 0x04xx, got 0x%04lx)\n", device, tty.st_rdev); 

	char vcsa_name[13] = "/dev/vcsa"; // /dev/vcsa + up to 3 digits + nul
	if(tty.st_rdev & 0xFF)
		sprintf(vcsa_name, "/dev/vcsa%ld", tty.st_rdev & 0xFF);

	int vcsa_desc = open(vcsa_name, O_RDONLY);
	if(vcsa_desc < 0)
		die("vcsa_init: Could not open VCSA device '%s' for reading: %s\n", vcsa_name, strerror(errno));
    say("vcsa_init: Opened the VCSA device '%s'\n", vcsa_name);

	struct vcsa_header header;
	read_all(vcsa_desc, &header, sizeof(struct vcsa_header));
	say("vcsa_init: Got VCSA device: %d lines %d columns\n", header.lines, header.columns);

	struct console_font_op op;
	op.op = KD_FONT_OP_GET;
	op.flags = 0;
	op.charcount = 0x200;
	op.width = 32;
	op.height = 32;
	op.data = calloc(0x200, 32 * 32 / 8);
	if(0 > ioctl(file_desc, KDFONTOP, &op))
		die("vcsa_init: KD_FONT_OP_GET failed: %s\n", strerror(errno));
	say("vcsa_init: Got font: %d glyphs, %dx%d\n", op.charcount, op.width, op.height);

	vcsa_userdata *userdata = calloc(1, sizeof(vcsa_userdata));
	userdata->file_desc = file_desc;
	userdata->vcsa_desc = vcsa_desc;
	userdata->header = header;
	userdata->data = op.data;

	descriptor desc;
	desc.width = op.width * header.columns;
	desc.height = op.height * header.lines;
	desc.userdata = userdata;

	return desc;
}

void vcsa_cleanup(descriptor const *desc)
{
	say("vcsa_cleanup: Cleaning up the VCSA device\n");
	close(((vcsa_userdata *)desc->userdata)->file_desc);
	close(((vcsa_userdata *)desc->userdata)->vcsa_desc);
	free(((vcsa_userdata *)desc->userdata)->data);
	free(desc->userdata);
	say("vcsa_cleanup: Done cleaning up\n");
}

static uint8_t colors_red[16] =   {  0,  0,  0,  0,192,192,192,192, 64, 64, 64, 64,255,255,255,255};
static uint8_t colors_green[16] = {  0,  0,192,192,  0,  0,192,192, 64, 64,255,255, 64, 64,255,255};
static uint8_t colors_blue[16] =  {  0,192,  0,192,  0,192,  0,192, 64,255, 64,255, 64,255, 64,255};

void vcsa_capture(descriptor const *desc, buffer buf)
{
	int file_desc = ((vcsa_userdata *)desc->userdata)->file_desc;
	int vcsa_desc = ((vcsa_userdata *)desc->userdata)->vcsa_desc;

	int width = desc->width;
	int height = desc->height;

	struct console_font_op op;
	op.op = KD_FONT_OP_GET;
	op.flags = 0;
	op.charcount = 0x200;
	op.width = 32;
	op.height = 32;
	op.data = (((vcsa_userdata *)desc->userdata)->data);
	if(0 > ioctl(file_desc, KDFONTOP, &op))
		die("vcsa_capture: KD_FONT_OP_GET failed: %s\n", strerror(errno));
	whisper("vcsa_capture: Got font: %d glyphs, %dx%d\n", op.charcount, op.width, op.height);

	uint16_t mask;
	if(0 > ioctl(file_desc, VT_GETHIFONTMASK, &mask))
		die("vcsa_capture: VT_GETHIFONTMASK failed: %s\n", strerror(errno));
	whisper("vcsa_capture: Got Unicode bitmask: 0x%04x\n", mask);

	struct vcsa_header header;
	lseek(vcsa_desc, 0, SEEK_SET);
	read_all(vcsa_desc, &header, sizeof(struct vcsa_header));

	uint16_t lower = mask - 1;
	size_t row_distance = (op.width + 7) / 8;
	size_t char_distance = row_distance * 32;
	uint16_t *data = calloc(header.columns * header.lines, sizeof(uint16_t));
	read_all(vcsa_desc, data, header.columns * header.lines * sizeof(uint16_t));
	int x, y;
	for(y = 0; y < height; y++)
	{
		int chary = y / op.height;
		int dy = y % op.height;
		uint16_t *line = data + header.columns * chary;
		for(x = 0; x < width; x++)
		{
			int charx = x / op.width;
			int dx = x % op.width;
			uint16_t ch = line[charx];
			int symbol = ch & 0xFF;
			if(ch & mask)
				symbol |= 0x100;
			if(lower)
				ch = ch & lower | ch >> 1 & ~lower;
			int sym_pixel = op.data[char_distance * symbol + row_distance * dy + (dx >> 3)] & (0x80 >> (dx & 0x7));
			int color = sym_pixel || (header.cursor_y == chary && header.cursor_x == charx && dy >= op.height - 2) ? (ch >> 8) & 0xF : ch >> 12;
			*(buf++) = colors_red[color];
			*(buf++) = colors_green[color];
			*(buf++) = colors_blue[color];
		}
	}
	free(data);
}
