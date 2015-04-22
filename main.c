#include <stdlib.h>
#include <getopt.h>

#include "ppm.h"
#include "reader_generic.h"
#include "reader_fb.h"
#include "log.h"

struct option options[] = {
	{"help", no_argument, 0, 'h'},
	{"debug", no_argument, 0, 'D'},
	{"device", required_argument, 0, 'd'},
	{"output", required_argument, 0, 'o'},
	{"quiet", no_argument, 0, 'q'},
	{"verbose", no_argument, 0, 'v'},
	{NULL, 0, 0, 0}
};

int main(int argc, char *argv[])
{
	char const *device = "/dev/fb0";
	char const *output = NULL;
	int help = 0;

	int arg, dummy;
	while(-1 != (arg = getopt_long(argc, argv, "hDd:o:qv", options, &dummy)))
		switch(arg)
		{
			case 'h':
			help = 1;
			break;
			case 'D':
			verbosity = 3;
			break;
			case 'd':
			device = optarg;
			break;
			case 'o':
			output = optarg;
			break;
			case 'q':
			verbosity = 0;
			break;
			case 'v':
			verbosity = 2;
			break;
		}
	if(optind < argc || help)
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "    vtshot -o <file> [-d <device>] [-qvDh]\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  -o <file>   --output <file>    Set the output file.\n");
		fprintf(stderr, "  -d <device> --device <device>  Set the input device (default: /dev/fb0).\n");
		fprintf(stderr, "  -q          --quiet            Suppress error messages.\n");
		fprintf(stderr, "  -v          --verbose          Show more informational messages.\n");
		fprintf(stderr, "  -D          --debug            Show debug information.\n");
		fprintf(stderr, "  -h          --help             Show this help.\n");
		return EXIT_FAILURE;
	}
	if(!output)
	{
		yell("No output filename specified. The -o/--output option is required. See --help for more info.\n");
		panic();
	}
	say("Done parsing options\n");
	descriptor desc = fb_init(device);
	buffer buf = calloc(1, desc.width * desc.height * 3);
	fb_capture(&desc, buf);
	write_ppm(output, desc.width, desc.height, buf);
	free(buf);
	fb_cleanup(&desc);
	say("Exiting with a success status code\n");
	return EXIT_SUCCESS;
}
