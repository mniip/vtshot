#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "reader_generic.h"
#include "fb.h"
#include "log.h"

int do_mmap = 0;

descriptor fb_init(char const *device)
{
	say("fb_init: Opening framebuffer device '%s'\n", device);
	int file_desc = open(device, O_RDONLY);
	if(file_desc < 0)
		die("fb_init: Could not open framebuffer device '%s' for reading: %s\n", device, strerror(errno));
	say("fb_init: Opened the framebuffer device\n");
	struct fb_fix_screeninfo fixed_info;
	if(0 > ioctl(file_desc, FBIOGET_FSCREENINFO, &fixed_info))
		die("fb_init: Could not get fixed screen info: %s\n", strerror(errno));
	say("fb_init: Got fixed screen info: framebuffer size = 0x%x, line length = 0x%x\n", fixed_info.smem_len, fixed_info.line_length);
	void *map = NULL;
	if(do_mmap)
	{
		if((void *)-1 == (map = mmap(NULL, fixed_info.smem_len, PROT_READ, MAP_PRIVATE, file_desc, 0)))
			die("fb_init: Could not mmap the framebuffer device: %s\n", strerror(errno));
		say("fb_init: Mmapped the framebuffer device\n");
	}
	struct fb_var_screeninfo screen_info;
	if(0 > ioctl(file_desc, FBIOGET_VSCREENINFO, &screen_info))
		die("fb_init: Could not get screen info: %s\n", strerror(errno));
	say("fb_init: Got screen info: width = %d, height = %d\n", screen_info.xres, screen_info.yres);

	fb_userdata *userdata = calloc(1, sizeof(fb_userdata));
	userdata->file_desc = file_desc;
	userdata->map = map;
	userdata->map_length = fixed_info.smem_len;

	descriptor desc;
	desc.width = screen_info.xres;
	desc.height = screen_info.yres;
	desc.userdata = userdata;
	
	return desc;
}

void fb_cleanup(descriptor const *desc)
{
	say("fb_cleanup: Cleaning up the framebuffer device\n");
	if(do_mmap)
		munmap(((fb_userdata *)desc->userdata)->map, ((fb_userdata *)desc->userdata)->map_length);
	close(((fb_userdata *)desc->userdata)->file_desc);
	free(desc->userdata);
	say("fb_cleanup: Done cleaning up\n");
}

static void read_all(int file_desc, void *buf, size_t size)
{
	while(size)
	{
		int ret;
		if(0 > (ret = read(file_desc, buf, size)))
			die("fb_capture: Error while reading from framebuffer device at offset 0x%lx: %s\n", lseek(file_desc, 0, SEEK_CUR), strerror(errno));
		if(!ret)
			say("fb_capture: Read ended early at offset 0x%lx\n", lseek(file_desc, 0, SEEK_CUR));
		size -= ret;
		buf = (uint8_t *)buf + ret;
	}
}

void fb_capture(descriptor const *desc, buffer buf)
{
	int file_desc = ((fb_userdata *)desc->userdata)->file_desc;
	struct fb_fix_screeninfo fixed_info;
	if(0 > ioctl(file_desc, FBIOGET_FSCREENINFO, &fixed_info))
		die("fb_capture: Could not get fixed screen info: %s\n", strerror(errno));
	struct fb_var_screeninfo screen_info;
	if(0 > ioctl(file_desc, FBIOGET_VSCREENINFO, &screen_info))
		die("fb_capture: Could not get screen info: %s\n", strerror(errno));
	int width = desc->width;
	int height = desc->height;
	if(width != screen_info.xres || height != screen_info.yres)
		yell("fb_capture: Device resolution changed\n");
	uint8_t *map = NULL;
	uint8_t *data = NULL;
	if(do_mmap)
		map = ((fb_userdata *)desc->userdata)->map;
	else
	{
		data = calloc(fixed_info.line_length * height, 1);
		lseek(file_desc, fixed_info.line_length * screen_info.yoffset, SEEK_SET);
		read_all(file_desc, data, fixed_info.line_length * height);
	}
	int x, y;
#define LOOP(X) \
	for(y = 0; y < height; y++)\
	{\
		uint8_t *line;\
		if(do_mmap)\
			line = map + fixed_info.line_length * (y + screen_info.yoffset);\
		else\
			line = data + fixed_info.line_length * y;\
		for(x = screen_info.xoffset; x < screen_info.xoffset + width; x++)\
		{\
			X\
		}\
	}
#define GENERIC_ASSIGN(pixel) \
		if(screen_info.red.length == 8)\
			*(buf++) = pixel >> screen_info.red.offset & 0xFF;\
		else\
		{\
			uint32_t red_mask = (1 << screen_info.red.length) - 1;\
			*(buf++) = (pixel >> screen_info.red.offset & red_mask) * 255 / red_mask;\
		}\
		if(screen_info.green.length == 8)\
			*(buf++) = pixel >> screen_info.green.offset & 0xFF;\
		else\
		{\
			uint32_t green_mask = (1 << screen_info.green.length) - 1;\
			*(buf++) = (pixel >> screen_info.green.offset & green_mask) * 255 / green_mask;\
		}\
		if(screen_info.blue.length == 8)\
			*(buf++) = pixel >> screen_info.blue.offset & 0xFF;\
		else\
		{\
			uint32_t blue_mask = (1 << screen_info.blue.length) - 1;\
			*(buf++) = (pixel >> screen_info.blue.offset & blue_mask) * 255 / blue_mask;\
		}

	whisper("fb_capture: Translating %d bits per pixel %dx%d pixmap\n", screen_info.bits_per_pixel, width, height);
	switch(screen_info.bits_per_pixel)
	{
		case 1:
		case 2:
		case 4:
		default:
		die("fb_capture: Capturing for %d bits per pixel not implemented yet\n", screen_info.bits_per_pixel);
		break;
		case 8:
		LOOP
		(
			uint8_t pixel = line[x];
			GENERIC_ASSIGN(pixel)
		)
		break;
		case 16:
		LOOP
		(
			uint16_t pixel = *(uint16_t *)(line + x * 2);
			GENERIC_ASSIGN(pixel)
		)
		break;
		case 24:
		LOOP
		(
			uint32_t pixel = *(uint16_t *)(line + x * 3) | *(uint8_t *)(line + x * 3 + 2) << 16;
			GENERIC_ASSIGN(pixel)
		)
		break;
		case 32:
		LOOP
		(
			uint32_t pixel = *(uint32_t *)(line + x * 4);
			GENERIC_ASSIGN(pixel)
		)
		break;
	}
	if(!do_mmap)
		free(data);
	whisper("fb_capture: Done translating\n");
}
