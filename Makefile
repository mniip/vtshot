CC= gcc
CFLAGS= -O3
LDFLAGS= -lrt -lpng -lgif

OUTFILE= vtshot
OBJECTS= fb.o gif.o log.o main.o png.o ppm.o rle.o vcsa.o writer_generic.o

all: vtshot

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OUTFILE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(OUTFILE) $+ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(OUTFILE)

fb.o: fb.h log.h reader_generic.h
gif.o: gif.h
log.o: log.h
main.o: fb.h gif.h log.h png.h ppm.h reader_generic.h rle.h vcsa.h writer_generic.h
png.o: log.h png.h reader_generic.h writer_generic.h
ppm.o: log.h ppm.h reader_generic.h writer_generic.h
rle.o: log.h rle.h
vcsa.o: log.h reader_generic.h vcsa.h
writer_generic.o: log.h rle.h reader_generic.h writer_generic.h

.PHONY: all clean
