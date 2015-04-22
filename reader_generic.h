#ifndef READER_GENERIC_H
#define READER_GENERIC_H

typedef struct
{
	int width, height;
	void *userdata;
}
descriptor;

typedef unsigned char *buffer;

typedef descriptor (*init_proc)(char const *filename);
typedef void (*cleanup_proc)(descriptor const *desc);
typedef void (*capture_proc)(descriptor const *desc, buffer buf);

#endif
