#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#include "png.h"
#include "ppm.h"
#include "reader_generic.h"
#include "reader_fb.h"
#include "reader_vcsa.h"
#include "log.h"

struct option options[] = {
	{"benchmark", no_argument, 0, 'b'},
	{"fb", no_argument, 0, 'f'},
	{"help", no_argument, 0, 'h'},
	{"debug", no_argument, 0, 'D'},
	{"device", required_argument, 0, 'd'},
	{"mmap", no_argument, 0, 'm'},
	{"output", required_argument, 0, 'o'},
	{"ppm", no_argument, 0, 'P'},
	{"png", no_argument, 0, 'p'},
	{"quiet", no_argument, 0, 'q'},
	{"vcsa", no_argument, 0, 'V'},
	{"verbose", no_argument, 0, 'v'},
	{NULL, 0, 0, 0}
};

int main(int argc, char *argv[])
{
	char const *device = NULL, *output = NULL;
	int help = 0, benchmark = 0, outtype = 0;
	char const *default_device = "/dev/fb0";
	init_proc init = &fb_init;
	cleanup_proc cleanup = &fb_cleanup;
	capture_proc capture = &fb_capture;

	int arg, dummy;
	while(-1 != (arg = getopt_long(argc, argv, "bfhDd:mo:PpqVv", options, &dummy)))
		switch(arg)
		{
			case 'b': benchmark = 1; break;
			case 'f': default_device = "/dev/fb0"; init = &fb_init; cleanup = &fb_cleanup; capture = &fb_capture; break;
			case 'h': help = 1; break;
			case 'D': verbosity = 3; break;
			case 'd': device = optarg; break;
			case 'm': do_mmap = 1; break;
			case 'P': outtype = 1; break;
			case 'p': outtype = 0; break;
			case 'o': output = optarg; break;
			case 'q': verbosity = 0; break;
			case 'V': default_device = "/dev/tty0"; init = &vcsa_init; cleanup = &vcsa_cleanup; capture = &vcsa_capture; break;
			case 'v': verbosity = 2; break;
		}
	if(optind < argc || help)
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "    vtshot (-o <file> | -b) [-d <device>] [-bfDhmPpqVv]\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  -o | --output <file>    Set the output file.\n");
		fprintf(stderr, "  -d | --device <device>  Set the input device (default: /dev/fb0 or /dev/tty0).\n");
		fprintf(stderr, "  -b | --benchmark        Do not capture the image, provide timing information instead.\n");
		fprintf(stderr, "  -m | --mmap             Use memory mapping (slower on some machines, faster on others).\n");
		fprintf(stderr, "  -p | --png              Set the output format to PNG (default).\n");
		fprintf(stderr, "  -P | --ppm              Set the output format to PPM.\n");
		fprintf(stderr, "  -f | --fb               Capture input from a framebuffer device.\n");
		fprintf(stderr, "  -V | --vcsa             Capture input from a VCSA device.\n");
		fprintf(stderr, "  -q | --quiet            Suppress error messages.\n");
		fprintf(stderr, "  -v | --verbose          Show more informational messages.\n");
		fprintf(stderr, "  -D | --debug            Show debug information.\n");
		fprintf(stderr, "  -h | --help             Show this help.\n");
		return EXIT_FAILURE;
	}
	if(!output && !benchmark)
		die("No output filename specified. The -o/--output option is required. See --help for more info.\n");
	if(!device)
		device = default_device;
	say("Done parsing options\n");
	descriptor desc = init(device);
	buffer buf = calloc(desc.width * desc.height * 3, 1);
	if(benchmark)
	{
		struct timespec start;
		struct timespec end;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		capture(&desc, buf);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		yell("Screenshot took %ld.%09ld seconds.\n", end.tv_sec - start.tv_sec + (end.tv_sec > start.tv_sec), (end.tv_nsec - start.tv_nsec) % 1000000000);
		double took = (double)(end.tv_sec - start.tv_sec) + (double)((end.tv_nsec - start.tv_nsec) % 1000000000) / 1.0e9;
		int takes = 3.0 / took;
		int i;
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		for(i = 0; i < takes; i++)
			capture(&desc, buf);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		double total = (double)(end.tv_sec - start.tv_sec) + (double)((end.tv_nsec - start.tv_nsec) % 1000000000) / 1.0e9;
		yell("%d screenshots took %.3lf seconds (average %.3lf seconds, %.3lf FPS).\n", takes, total, total / takes, takes / total);
	}
	else
	{
		capture(&desc, buf);
		switch(outtype)
		{
			case 0: write_png(output, desc.width, desc.height, buf); break;
			case 1: write_ppm(output, desc.width, desc.height, buf); break;
		}
	}
	free(buf);
	cleanup(&desc);
	say("Exiting with a success status code\n");
	return EXIT_SUCCESS;
}
