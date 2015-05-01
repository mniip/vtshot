#include <string.h>

#include <gif_lib.h>

#include "gif.h"
#include "log.h"
#include "reader_generic.h"
#include "rle.h"
#include "writer_generic.h"

#if GIFLIB_MAJOR >= 5
#define MakeMapObject GifMakeMapObject
#define FreeMapObject GifFreeMapObject
#define QuantizeBuffer GifQuantizeBuffer
#define EGifOpenFileName(name, test) EGifOpenFileName(name, test, NULL)
#if GIFLIB_MINOR >= 1
#define EGifCloseFile(gif) EGifCloseFile(gif, NULL)
#endif
#endif

extern double fps;

void write_gif(char const *filename, int width, int height, buffer buf)
{
	say("write_gif: Writing %dx%d GIF to '%s'\n", width, height, filename);
	GifFileType *gif = EGifOpenFileName(filename, 0);
	if(!gif)
		die("write_gif: Unable to open GIF file '%s'\n", filename);
	int palette_size = 256;
	ColorMapObject *global_palette = MakeMapObject(palette_size, NULL);
	if(!global_palette)
		die("write_gif: Unable to create the global palette\n");
	uint8_t *r = calloc(width * height, 1), *g = calloc(width * height, 1), *b = calloc(width * height, 1);
	uint8_t *frame = calloc(width * height, 1);
	size_t i, j;
	for(i = 0, j = 0; i < width * height; i++)
	{
		r[i] = buf[j++];
		g[i] = buf[j++];
		b[i] = buf[j++];
	}
	if(GIF_ERROR == QuantizeBuffer(width, height, &palette_size, r, g, b, frame, global_palette->Colors))
		die("write_gif: Unable to quantize\n");
	if(GIF_ERROR == EGifPutScreenDesc(gif, width, height, 8, 0, global_palette))
		die("write_gif: Unable to write GIF descriptor\n");
	FreeMapObject(global_palette);
	say("write_gif: Wrote the header\n");

	if(GIF_ERROR == EGifPutImageDesc(gif, 0, 0, width, height, 0, NULL))
		die("write_gif: Unable to write the local image descriptor\n");
	if(GIF_ERROR == EGifPutLine(gif, frame, width * height))
		die("write_gif: Unable to dump the buffer\n");
	free(r);
	free(g);
	free(b);
	free(frame);
	if(GIF_ERROR == EGifCloseFile(gif))
		die("write_gif_sequence: Unable to close the GIF file\n");
	say("write_gif: Done writing\n");
}

void write_gif_sequence(char const *filename, int width, int height, sequence *head)
{
	say("write_gif_sequence: Writing %dx%d animated GIF to '%s'\n", width, height, filename);
	GifFileType *gif = EGifOpenFileName(filename, 0);
	if(!gif)
		die("write_gif_sequence: Unable to open GIF file '%s'\n", filename);
	ColorMapObject *global_palette = MakeMapObject(2, NULL);
	if(!global_palette)
		die("write_gif_sequence: Unable to create the global palette\n");
	if(GIF_ERROR == EGifPutScreenDesc(gif, width, height, 8, 0, global_palette))
		die("write_gif_sequence: Unable to write GIF descriptor\n");
	FreeMapObject(global_palette);
	say("write_gif_sequence: Wrote the header\n");

	char nsle[] = "NETSCAPE2.0";
	char animation[3] = {1, 0, 0};
#if GIFLIB_MAJOR >= 5
	if(GIF_ERROR == EGifPutExtensionLeader(gif, APPLICATION_EXT_FUNC_CODE))
		die("write_gif_sequence: Failed to start the NSLE extension\n");
	if(GIF_ERROR == EGifPutExtensionBlock(gif, strlen(nsle), nsle))
		die("write_gif_sequence: Failed to add the NSLE extension\n");
	if(GIF_ERROR == EGifPutExtensionBlock(gif, sizeof(animation), animation))
		die("write_gif_sequence: Failed to add the animation extension\n");
	if(GIF_ERROR == EGifPutExtensionTrailer(gif))
		die("write_gif_sequence: Failed to finish the NSLE extension\n");
#else
	if(GIF_ERROR == EGifPutExtensionFirst(gif, APPLICATION_EXT_FUNC_CODE, strlen(nsle), nsle))
		die("write_gif_sequence: Failed to add the NSLE extension\n");
	if(GIF_ERROR == EGifPutExtensionLast(gif, APPLICATION_EXT_FUNC_CODE, sizeof(animation), animation))
		die("write_gif_sequence: Failed to add the animation extension\n");
#endif
	say("write_gif_sequence: Wrote the animation block\n");

	int frames = 0;
	buffer buf = calloc(width * height, 3), buf_prev = calloc(width * height, 3);
	uint8_t *r = calloc(width * height, 1), *g = calloc(width * height, 1), *b = calloc(width * height, 1);
	uint8_t *frame = calloc(width * height, 1);
	char *trans = calloc(width * height, 1);
	while(head->next)
	{
		rle_free(head->rle, width * height, buf);
		int x_min = width, x_max = -1, y_min = height, y_max = -1;
		int x, y;
		size_t i;
		i = 0;
		for(y = 0; y < height; y++)
			for(x = 0; x < width; x++, i++)
			{
				uint32_t a = *(uint16_t *)(buf + i * 3) | *(uint8_t *)(buf + i * 3 + 2) << 16;
				uint32_t b = *(uint16_t *)(buf_prev + i * 3) | *(uint8_t *)(buf_prev + i * 3 + 2) << 16;
				if(!frames || a != b)
				{
					if(x < x_min)
						x_min = x;
					if(x > x_max)
						x_max = x;
					if(y < y_min)
						y_min = y;
					if(y > y_max)
						y_max = y;
					trans[i] = 0;
				}
				else
					trans[i] = 1;
			}
		if(x_min > x_max)
			x_min = x_max = 0;
		if(y_min > y_max)
			y_min = y_max = 0;
		i = 0;
		for(y = y_min; y <= y_max; y++)
			for(x = x_min; x <= x_max; x++, i++)
				if(trans[y * width + x])
				{
					r[i] = g[i] = b[i] = 0;
				}
				else
				{
					r[i] = buf[(y * width + x) * 3];
					g[i] = buf[(y * width + x) * 3 + 1];
					b[i] = buf[(y * width + x) * 3 + 2];
				}
		int x_size = x_max - x_min + 1, y_size = y_max - y_min + 1;
		whisper("write_gif_sequence: Only updating %dx%d region at %dx%d\n", x_size, y_size, x_min, y_min);
		int palette_size = 256;
		ColorMapObject *local_palette = MakeMapObject(palette_size, NULL);
		if(!local_palette)
			die("write_gif_sequence: Unable to create the local palette at frame %d\n", frames);
		palette_size = 255;
		if(GIF_ERROR == QuantizeBuffer(x_size, y_size, &palette_size, r, g, b, frame, local_palette->Colors))
			die("write_gif_sequence: Unable to quantize frame %d\n", frames);
		if(palette_size >= 256)
			die("write_gif_sequence: Quantization didn't reserve a slot for transparency at frame %d\n", frames);

		int msec = 100 / fps;
		char gce[4] = {1, msec & 0xFF, msec >> 8, palette_size};
		if(GIF_ERROR == EGifPutExtension(gif, GRAPHICS_EXT_FUNC_CODE, sizeof(gce), gce))
			die("write_gif_sequence: Failed to add the GCE extension at frame %d\n", frames);

		i = 0;
		for(y = y_min; y <= y_max; y++)
			for(x = x_min; x <= x_max; x++, i++)
				if(trans[y * width + x])
					frame[i] = palette_size;

		if(GIF_ERROR == EGifPutImageDesc(gif, x_min, y_min, x_size, y_size, 0, local_palette))
			die("write_gif_sequence: Unable to write the local image descriptor at frame %d\n", frames);
		FreeMapObject(local_palette);
		if(GIF_ERROR == EGifPutLine(gif, frame, x_size * y_size))
			die("write_gif_sequence: Unable to dump the buffer at frame %d\n", frames);

		buffer t = buf;
		buf = buf_prev;
		buf_prev = t;

		frames++;
		sequence *next = head->next;
		free(head);
		head = next;
	}
	free(head);
	free(buf); free(buf_prev);
	free(r); free(g); free(b);
	free(frame);
	free(trans);
	if(GIF_ERROR == EGifCloseFile(gif))
		die("write_gif_sequence: Unable to close the GIF file\n");
	say("write_gif_sequence: Wrote %d frames to '%s'\n", frames, filename);
}
