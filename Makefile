CC= gcc
CFLAGS= -O0 -ggdb
LDFLAGS= -lrt -lpng

OUTFILE= vtshot
OBJECTS= log.o main.o png.o ppm.o reader_fb.o

all: main

%.o: %.c %.T
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

main: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(OUTFILE) $+

clean:
	rm -f $(OBJECTS) $(OUTFILE)

main.T: log.h png.h ppm.h reader_fb.h reader_generic.h
png.T: log.h png.h reader_generic.h
ppm.T: log.h ppm.h reader_generic.h
reader_fb.T: log.h reader_fb.h reader_generic.h

.PHONY: all main.T png.T ppm.T reader_fb.T
