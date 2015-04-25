#ifndef FB_H
#define FB_H

#include "reader_generic.h"

extern int do_mmap;

typedef struct
{
	int file_desc;
	void *map;
	size_t map_length;
}
fb_userdata;

descriptor fb_init(char const *filename);
extern void fb_cleanup(descriptor const *desc);
extern void fb_capture(descriptor const *desc, buffer buf);

#endif
