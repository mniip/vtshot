#ifndef RLE_H
#define RLE_H

#include <stdint.h>

#include "reader_generic.h"

typedef uint32_t *rle;

rle rle_allocate(size_t size, buffer buf);
void rle_free(rle rel, size_t size, buffer buf);

#endif
