ifdef DEBUG
CFLAGS = -O0 -ggdb
else
CFLAGS = -Os -s -fomit-frame-pointer
endif

TARGETS=bin/netelf bin/test bin/test_loaddll

all: $(TARGETS)

bin/netelf: netelf.c
	$(CC) $(CFLAGS) -o $@ $^ -lrt

bin/test: test.c
	$(CC) $(CFLAGS) -o $@ $^

bin/test_loaddll: test_loaddll.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
