#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>

#include <signal.h>
#include <sys/wait.h>

#include "fb.h"
#include "gif.h"
#include "log.h"
#include "png.h"
#include "ppm.h"
#include "reader_generic.h"
#include "rle.h"
#include "vcsa.h"

struct option options[] = {
	{"benchmark", no_argument, 0, 'b'},
	{"debug", no_argument, 0, 'D'},
	{"device", required_argument, 0, 'd'},
	{"fps", required_argument, 0, 'F'},
	{"fb", no_argument, 0, 'f'},
	{"gif", no_argument, 0, 'g'},
	{"help", no_argument, 0, 'h'},
	{"mmap", no_argument, 0, 'm'},
	{"ppm", no_argument, 0, 'P'},
	{"png", no_argument, 0, 'p'},
	{"quiet", no_argument, 0, 'q'},
	{"shell", no_argument, 0, 'S'},
	{"sequence", no_argument, 0, 's'},
	{"vcsa", no_argument, 0, 'V'},
	{"verbose", no_argument, 0, 'v'},
	{NULL, 0, 0, 0}
};

static int terminated = 0;

static void handler(int r)
{
	signal(SIGINT, SIG_DFL);
	terminated = 1;
}

double fps = 24.0;

int main(int argc, char *argv[])
{
	char const *device = NULL, *output = NULL;
	int help = 0, benchmark = 0, seq = 0;
	char const *default_device = "/dev/fb0";
	init_proc init = &fb_init;
	cleanup_proc cleanup = &fb_cleanup;
	capture_proc capture = &fb_capture;
	write_proc write = &write_png;
	write_proc_sequence write_sequence = &write_png_sequence;

	int arg, dummy;
	while(-1 != (arg = getopt_long(argc, argv, "bDd:F:fghmPpqSsVv", options, &dummy)))
		switch(arg)
		{
			case 'b': benchmark = 1; break;
			case 'D': verbosity = 3; break;
			case 'd': device = optarg; break;
			case 'F': if(!sscanf(optarg, "%lf", &fps)) die("Not a number: '%s'\n", optarg); break;
			case 'f': default_device = "/dev/fb0"; init = &fb_init; cleanup = &fb_cleanup; capture = &fb_capture; break;
			case 'g': write = &write_gif; write_sequence = &write_gif_sequence; break;
			case 'h': help = 1; break;
			case 'm': do_mmap = 1; break;
			case 'P': write = &write_ppm; write_sequence = &write_ppm_sequence; break;
			case 'p': write = &write_png; write_sequence = &write_png_sequence; break;
			case 'q': verbosity = 0; break;
			case 'S': seq = 2; break;
			case 's': seq = 1; break;
			case 'V': default_device = "/dev/tty0"; init = &vcsa_init; cleanup = &vcsa_cleanup; capture = &vcsa_capture; break;
			case 'v': verbosity = 2; break;
		}
	if(optind < argc)
		output = argv[optind++];
	if(optind < argc || help)
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "    vtshot (<file> | -b) [-d <device>] [-f <fps>] [-bDdFghmPpqVv]\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  -d | --device <file>    Set the input device (default: /dev/fb0 or /dev/tty0).\n");
		fprintf(stderr, "  -f | --fb               Capture input from a framebuffer device (default).\n");
		fprintf(stderr, "  -V | --vcsa             Capture input from a VCSA device.\n");
		fprintf(stderr, "  -m | --mmap             Use memory mapping (slower on some machines, faster on others).\n");
		fprintf(stderr, "  -b | --benchmark        Do not capture the image, provide timing information instead.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  -p | --png              Set the output format to PNG (default).\n");
		fprintf(stderr, "  -P | --ppm              Set the output format to PPM.\n");
		fprintf(stderr, "  -g | --gif              Set the output format to GIF.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  -s | --sequence         Record an animation, terminated by SIGINT (^C).\n");
		fprintf(stderr, "  -S | --shell            Launch $SHELL and record an animation, until the shell exits.\n");
		fprintf(stderr, "  -F | --fps <number>     Set the animation FPS (default: 24.0).\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  -q | --quiet            Suppress error messages.\n");
		fprintf(stderr, "  -v | --verbose          Show more informational messages.\n");
		fprintf(stderr, "  -D | --debug            Show debug information.\n");
		fprintf(stderr, "  -h | --help             Show this help.\n");
		return EXIT_FAILURE;

	}
	if(!output && !benchmark)
		die("No output filename specified. See --help for more info.\n");
	if(!device)
		device = default_device;
	say("Done parsing options\n");
	descriptor desc = init(device);
	buffer buf = calloc(desc.width * desc.height * 3, 1);
	if(benchmark)
	{
		struct timespec start, end;
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
	else if(seq)
	{
		sequence *head = calloc(1, sizeof(sequence));
		sequence *tail = head;
		if(seq == 1)
			signal(SIGINT, &handler);
		int pid = 0;
		if(seq == 2)
		{
			say("Clearing screen\n");
			fflush(stderr);
			fprintf(stdout, "\033[2J\033[H");
			fflush(stdout);
			int p = fork();
			if(p < 0)
				die("Fork failed: %s\n", strerror(errno));
			if(p)
			{
				pid = p;
				say("Forked to pid %d\n", pid);
			}
			else
			{
				cleanup(&desc);
				char const *sh = getenv("SHELL");
				say("Launching shell '%s'\n", sh);
				if(0 > execl(sh, sh, NULL))
					die("Exec of '%s' failed: %s\n", sh, strerror(errno));
			}
		}
		while(!terminated)
		{
			struct timespec start, end;
			clock_gettime(CLOCK_MONOTONIC, &start);
			capture(&desc, buf);
			tail->rle = rle_allocate(desc.width * desc.height, buf);
			tail = tail->next = calloc(1, sizeof(sequence));
			clock_gettime(CLOCK_MONOTONIC, &end);
			uint64_t spent = (end.tv_nsec - start.tv_nsec) % 1000000000 + 1000000000 * (end.tv_sec - start.tv_sec);
			int64_t remaining = (uint64_t)(1000000000 / fps) - spent;
			if(remaining > 0)
			{
				struct timespec sleep;
				sleep.tv_nsec = remaining;
				sleep.tv_sec = 0;
				nanosleep(&sleep, &sleep);
			}
			else
				say("Underrun\n");
			if(seq == 2)
			{
				int ret = waitpid(pid, NULL, WNOHANG);
				if(ret < 0)
					die("Waitpid for pid %d failed: %s\n", pid, strerror(errno));
				if(ret == pid)
				{
					say("Child shell exited\n");
					terminated = 1;
				}
			}
		}
		cleanup(&desc);
		say("Starting encoding\n");
		write_sequence(output, desc.width, desc.height, head);
	}
	else
	{
		capture(&desc, buf);
		write(output, desc.width, desc.height, buf);
		cleanup(&desc);
	}
	free(buf);
	say("Exiting with a success status code\n");
	return EXIT_SUCCESS;
}
