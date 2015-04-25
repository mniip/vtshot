#include <stdlib.h>

#include "log.h"
#include "reader_generic.h"
#include "rle.h"

rle rle_allocate(size_t size, buffer buf)
{
	rle rle = calloc(size, sizeof(uint32_t));
	size_t i = 0;
	size_t current = 0;
	while(current < size * 3)
	{
		uint32_t base = *(uint16_t *)(buf + current) | *(uint8_t *)(buf + current + 2) << 16;
		int run = 0;
		do
		{
			current += 3;
			run++;
		}
		while(run < 0xFF && current < size * 3 && base == (*(uint16_t *)(buf + current) | *(uint8_t *)(buf + current + 2)) << 16);
		rle[i++] = base | run << 24;
	}
	whisper("rle_allocate: Shrunk %ld bytes to %ld.\n", size * 3, i * sizeof(uint32_t));
	return realloc(rle, i * sizeof(uint32_t));
}

void rle_free(rle rle, size_t size, buffer buf)
{
	size_t i = 0;
	while(size)
	{
		uint32_t sequence = rle[i++];
		int run = sequence >> 24;
		uint8_t r = sequence;
		uint8_t g = sequence >> 8;
		uint8_t b = sequence >> 16;
		if(size < run)
			die("rle_free: RLE buffer overflow.\n");
		size -= run;
		while(run--)
		{
			*(buf++) = r;
			*(buf++) = g;
			*(buf++) = b;
		}
	}
	free(rle);
}
