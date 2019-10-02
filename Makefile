CC=gcc
CFLAGS=

all: writenoncanonical noncanonical

writenoncanonical: writenoncanonical.c
	$(CC) $(CFLAGS) -c -o writenoncanonical writenoncanonical.c

noncanonical: noncanonical.c
	$(CC) $(CFLAGS) -c -o noncanonical noncanonical.c

clean:
	rm -f *.o *.d noncanonical writenoncanonical