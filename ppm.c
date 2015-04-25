#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>

#include "ppm.h"
#include "log.h"

static void write_all(int file_desc, void *data, size_t size)
{
	size_t orig_size = size;
	while(size > 0)
	{
		int ret;
		if(0 > (ret = write(file_desc, data, size)))
			die("write_ppm: Error while writing PPM: %s\n", strerror(errno));
		size -= ret;
	}
}

void write_ppm(char const *filename, int width, int height, buffer buf)
{
	whisper("write_ppm: Writing %dx%d PPM to '%s'\n", width, height, filename);
	int file_desc = open(filename, O_WRONLY | O_CREAT, 0644);
	if(file_desc < 0)
		die("write_ppm: Could not open '%s' for writing: %s\n", filename, strerror(errno));
	whisper("write_ppm: Opened the file\n");
	char *header = calloc(20, 1); // 11 bytes of text, plus 2 numbers 9 each plus terminating null
	sprintf(header, "P6\n#\n%d %d\n255\n", width, height);
	write_all(file_desc, header, strlen(header));
	whisper("write_ppm: Wrote the header\n");
	write_all(file_desc, buf, width * height * 3);
	whisper("write_ppm: Wrote the buffer\n");
	close(file_desc);
	whisper("write_ppm: Done writing\n");
}

extern void write_ppm_sequence(char const *basename, int width, int height, sequence *head)
{
	write_file_sequence(basename, width, height, head, &write_ppm);
}
