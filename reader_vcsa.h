#ifndef READER_VCSA_H
#define READER_VCSA_H

#include <stdint.h>

#include "reader_generic.h"

struct vcsa_header
{
	uint8_t lines, columns;
	uint8_t cursor_x, cursor_y;
};


typedef struct
{
	int file_desc, vcsa_desc;
	struct vcsa_header header;
	void *data;
}
vcsa_userdata;

descriptor vcsa_init(char const *filename);
extern void vcsa_cleanup(descriptor const *desc);
extern void vcsa_capture(descriptor const *desc, buffer buf);

#endif
