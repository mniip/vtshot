#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <png.h>

#include "png.h"
#include "log.h"

static void write_all(int file_desc, void *data, size_t size)
{
	while(size > 0)
	{
		int ret;
		if(0 > (ret = write(file_desc, data, size)))
		{
			yell("write_ppm: Error while writing PNG: %s\n", strerror(errno));
			panic();
		}
		size -= ret;
	}
}

static void png_write_cb(png_structp png_ptr, png_bytep data, png_size_t length)
{
	int file_desc = *(int *)png_get_io_ptr(png_ptr);
	write_all(file_desc, data, length);
}

void write_png(char const *filename, int width, int height, buffer buf)
{
	whisper("write_png: Writing %dx%d PNG to '%s'\n", width, height, filename);
	int file_desc = open(filename, O_WRONLY | O_CREAT, 0644);
	if(file_desc < 0)
	{
		yell("write_png: Could not open '%s' for writing: %s\n", filename, strerror(errno));
		panic();
	}
	whisper("write_png: Opened the file\n");
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		yell("write_png: png_create_write_struct failed\n");
		panic();
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		yell("write_png: png_create_info_struct failed\n");
		panic();
	}
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_write_fn(png_ptr, &file_desc, &png_write_cb, NULL);
	void **row_pointers = calloc(height, sizeof(void *));
	int y;
	for(y = 0; y < height; y++)
		row_pointers[y] = buf + width * y * 3;
	png_set_rows(png_ptr, info_ptr, (png_bytepp)row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
	whisper("write_png: Wrote PNG\n");
	free(row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	close(file_desc);
	whisper("write_png: Done writing\n");
}
