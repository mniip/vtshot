#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"
#include "reader_generic.h"
#include "rle.h"
#include "writer_generic.h"

void write_file_sequence(char const *basename, int width, int height, sequence *head, write_proc write)
{
	int frame = 0;
	char filename[strlen(basename) + 8];
	buffer buf = calloc(width * height, 3);
	while(head->next)
	{
		sprintf(filename, "%s.%06d", basename, frame);
		rle_free(head->rle, width * height, buf);
		write(filename, width, height, buf);
		frame++;
		sequence *next = head->next;
		free(head);
		head = next;
	}
	free(head);
	say("write_file_sequence: Wrote %d frames to '%s.######'.\n", frame, basename);
}
