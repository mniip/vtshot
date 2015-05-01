#include <string.h>

#include <gif_lib.h>

#include "gif.h"
#include "log.h"
#include "reader_generic.h"
#include "rle.h"
#include "writer_generic.h"

extern double fps;

void write_gif(char const *filename, int width, int height, buffer buf)
{
	say("write_gif: Writing %dx%d GIF to '%s'\n", width, height, filename);
	GifFileType *gif = EGifOpenFileName(filename, FALSE);
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

	if(GIF_ERROR == EGifPutImageDesc(gif, 0, 0, width, height, FALSE, NULL))
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
	GifFileType *gif = EGifOpenFileName(filename, FALSE);
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
	if(GIF_ERROR == EGifPutExtensionFirst(gif, APPLICATION_EXT_FUNC_CODE, strlen(nsle), nsle))
		die("write_gif_sequence: Failed to add the NSLE extension\n");
	char animation[3] = {1, 0, 0};
	if(GIF_ERROR == EGifPutExtensionLast(gif, APPLICATION_EXT_FUNC_CODE, sizeof(animation), animation))
		die("write_gif_sequence: Failed to add the animation extension\n");
	say("write_gif_sequence: Wrote the animation block\n");

	int frames = 0;
	buffer buf = calloc(width * height, 3);
	uint8_t *r = calloc(width * height, 1), *g = calloc(width * height, 1), *b = calloc(width * height, 1);
	uint8_t *frame = calloc(width * height, 1);
	while(head->next)
	{
		int msec = 100 / fps;
		char gce[4] = {0, msec & 0xFF, msec >> 8, 0};
		if(GIF_ERROR == EGifPutExtension(gif, GRAPHICS_EXT_FUNC_CODE, sizeof(gce), gce))
			die("write_gif_sequence: Failed to add the delay extension at frame %d\n", frames);

		rle_free(head->rle, width * height, buf);
		size_t i, j;
		for(i = 0, j = 0; i < width * height; i++)
		{
			r[i] = buf[j++];
			g[i] = buf[j++];
			b[i] = buf[j++];
		}
		int palette_size = 256;
		ColorMapObject *local_palette = MakeMapObject(palette_size, NULL);
		if(!local_palette)
			die("write_gif_sequence: Unable to create the local palette at frame %d\n", frames);
		if(GIF_ERROR == QuantizeBuffer(width, height, &palette_size, r, g, b, frame, local_palette->Colors))
			die("write_gif_sequence: Unable to quantize frame %d\n", frames);
		if(GIF_ERROR == EGifPutImageDesc(gif, 0, 0, width, height, FALSE, local_palette))
			die("write_gif_sequence: Unable to write the local image descriptor at frame %d\n", frames);
		FreeMapObject(local_palette);
		if(GIF_ERROR == EGifPutLine(gif, frame, width * height))
			die("write_gif_sequence: Unable to dump the buffer at frame %d\n", frames);

		frames++;
		sequence *next = head->next;
		free(head);
		head = next;
	}
	free(head);
	free(buf);
	free(r);
	free(g);
	free(b);
	free(frame);
	if(GIF_ERROR == EGifCloseFile(gif))
		die("write_gif_sequence: Unable to close the GIF file\n");
	say("write_gif_sequence: Wrote %d frames to '%s'\n", frames, filename);
}
