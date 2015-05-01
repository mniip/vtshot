#ifndef GIF__H
#define GIF__H

#include "reader_generic.h"
#include "writer_generic.h"

void write_gif(char const *filename, int width, int height, buffer buf);
void write_gif_sequence(char const *filename, int width, int height, sequence *head);

#endif
