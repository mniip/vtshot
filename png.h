#ifndef PNG__H
#define PNG__H

#include "reader_generic.h"
#include "writer_generic.h"

extern void write_png(char const *filename, int width, int height, buffer buf);
extern void write_png_sequence(char const *basename, int width, int height, sequence *head);

#endif
