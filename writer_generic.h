#ifndef WRITER_GENERIC_H
#define WRITER_GENERIC_H

#include "reader_generic.h"
#include "rle.h"

typedef void (*write_proc)(char const *filename, int width, int height, buffer buf);

typedef struct sequence_s
{
	struct sequence_s *next;
	rle rle;
}
sequence;

typedef void (*write_proc_sequence)(char const *filename, int width, int height, sequence *head);

extern void write_file_sequence(char const *basename, int width, int height, sequence *head, write_proc write);

#endif
