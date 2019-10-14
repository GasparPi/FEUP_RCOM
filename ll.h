#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>	
#include <signal.h>

#define C0     0x00
#define C1     0x40

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
#define MAX_TIMEOUT 3
#define MAX_FRAME_SIZE 4
#define MAX_PACKET_SIZE (6 + MAX_FRAME_SIZE)

#define TRANSMITTER 0
#define RECEIVER 1

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

// aux ll functions
int startConnection(const char* port);
int stopConnection(int fd);
int dataStateMachine(enum state* connection_state, unsigned char read_byte);
int readPacket(int fd, unsigned char* buf[]);

// ll functions
int llopen(const char* port, int role);
int llclose(int fd, int role);
int llwrite(int fd, char* buf, int length);
int llread(int fd, unsigned char* buf);

// Transmitter
int readResponse(int fd, const unsigned char expected[]);
void alarmHandler();

// Receiver
int readCommand(int fd, const unsigned char expected[]);

enum state {
 	START,
 	FLAG_RCV,
 	A_RCV,
 	C_RCV,
 	BCC_OK,
 	DATA_RCV,
  	STOP
};


