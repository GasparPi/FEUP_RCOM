CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic

all: alarm app ll main

alarm: alarm.c
	$(CC) $(CFLAGS) -c -o alarm alarm.c

ll: ll.c
	$(CC) $(CFLAGS) -c -o ll ll.c

app: app.c
	$(CC) $(CFLAGS) -c -o app app.c


main: main.c
	$(CC) $(CFLAGS) -o main main.c

clean:
	rm -f *.o *.d alarm app ll main
