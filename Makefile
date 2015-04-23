CC= gcc
CFLAGS= -O3
LDFLAGS= -lrt -lpng

OUTFILE= vtshot
OBJECTS= log.o main.o png.o ppm.o reader_fb.o reader_vcsa.o

all: main

%.o: %.c %.T
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

main: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(OUTFILE) $+ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(OUTFILE)

log.T: log.h
main.T: log.h png.h ppm.h reader_fb.h reader_generic.h reader_vcsa.h
png.T: log.h png.h reader_generic.h
ppm.T: log.h ppm.h reader_generic.h
reader_fb.T: log.h reader_fb.h reader_generic.h
reader_vcsa.T: log.h reader_generic.h reader_vcsa.h

.PHONY: all log.T main.T png.T ppm.T reader_fb.T reader_vcsa.T
