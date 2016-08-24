CFLAGS=-Os -s -fomit-frame-pointer -Wall -pedantic

all: netelf

netelf: netelf.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt

clean:
	rm -f netelf
