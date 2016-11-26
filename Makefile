ifdef DEBUG
CFLAGS = -O0 -ggdb
else
CFLAGS = -Os -s -fomit-frame-pointer
endif

all: netelf

netelf: netelf.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt

clean:
	rm -f netelf
