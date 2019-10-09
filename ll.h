#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>	
#include <signal.h>

#define C1     0x40
#define C0     0x00

#define FLAG 0x7E
#define A_CMD 0x03
#define A_REP 0x01 // Not used yet :)

#define C_SET 	0x03
#define C_DISC 	0x0b
#define C_UA 	0x07
#define C_RR0	0x05
#define C_RR1	0x85
#define C_REJ0	0x81
#define C_REJ1	0x01

#define BCC(X, Y) (X) ^ (Y)

#define MAX_RETRIES 3
#define MAX_TIMEOUT 1

#define TRANSMITTER 0
#define RECEIVER 1

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

// aux ll functions
//int startConnection(const char* port);
//int stopConnection(int fd);

// ll functions
int llopen(const char* port, int role);
int llclose(int fd, int role);

// Transmitter
//int readResponse(int fd, const unsigned char expected[]);
void alarmHandler();

// Receiver
//int readCommand(int fd, const unsigned char expected[]);

enum state {
 	START,
 	FLAG_RCV,
 	A_RCV,
 	C_RCV,
 	BCC_OK,
  	STOP
};


