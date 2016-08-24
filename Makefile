CFLAGS=-Os -s -fomit-frame-pointer -Wall -pedantic

all: client_shm client_memfd

client_shm: client_shm.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt

client_memfd: client_memfd.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f client_memfd client_shm
