#ifndef LOG_H
#define LOG_H

#include <stdio.h>

extern int verbosity;

#define yell(...) if(verbosity > 0) fprintf(stderr, __VA_ARGS__)
#define say(...) if(verbosity > 1) fprintf(stderr, __VA_ARGS__)
#define whisper(...) if(verbosity > 2) fprintf(stderr, __VA_ARGS__)

extern void panic();

#endif
