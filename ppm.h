#ifndef PPM_H
#define PPM_H

#include "reader_generic.h"
#include "writer_generic.h"

extern void write_ppm(char const *filename, int width, int height, buffer buf);
extern void write_ppm_sequence(char const *basename, int width, int height, sequence *head);

#endif
