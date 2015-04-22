#include <stdlib.h>

#include "log.h"

int verbosity = 1;

void panic()
{
	whisper("log: exiting with a failure status code\n");
	exit(EXIT_FAILURE);
}
