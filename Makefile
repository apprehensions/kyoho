all: unarp

unarp: unarp.c
	$(CC) -o $@ unarp.c -std=c99 -pedantic -Wall $(CFLAGS) $(LDFLAGS) -larchive
