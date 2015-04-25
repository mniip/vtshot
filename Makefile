CC= gcc
CFLAGS= -O3
LDFLAGS= -lrt -lpng

OUTFILE= vtshot
OBJECTS= log.o main.o png.o ppm.o fb.o vcsa.o rle.o writer_generic.o

all: vtshot

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OUTFILE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(OUTFILE) $+ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(OUTFILE)

log.o: log.h
main.o: fb.h log.h png.h ppm.h reader_generic.h rle.h vcsa.h writer_generic.h
png.o: log.h png.h reader_generic.h writer_generic.h
ppm.o: log.h ppm.h reader_generic.h writer_generic.h
fb.o: fb.h log.h reader_generic.h
vcsa.o: log.h reader_generic.h vcsa.h
rle.o: log.h rle.h
writer_generic.o: log.h rle.h reader_generic.h writer_generic.h

.PHONY: all clean
