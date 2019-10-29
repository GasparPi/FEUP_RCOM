#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alarm.h"

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
#define CHECK_C(C) (C == C_SET || C == C_DISC || C == C_UA || C == C_RR0 || C == C_RR1 || C == C_REJ0 || C == C_REJ1) ? 1 : 0

#define ESC 0x7D
#define STUFFING 0x20

#define MAX_RETRIES 3
#define MAX_FRAME_SIZE 2500

#define TRANSMITTER 0
#define RECEIVER 1

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

enum state {
 	START,
 	FLAG_RCV,
 	A_RCV,
 	C_RCV,
 	BCC_OK,
 	DATA_RCV,
	STOP
};

typedef struct {
	unsigned int numSentIFrames;
	unsigned int numReceivedIFrames;

	unsigned int timeouts;

	unsigned int numSentRR;
	unsigned int numReceivedRR;

	unsigned int numSentREJ;
	unsigned int numReceivedREJ;
} Statistics;

typedef struct {
	// port name "/dev/ttySX"
	char port[20];
	// connection mode (receiver or transmitter)
	unsigned int mode;
	// transmission speed
	unsigned int baudrate;
	// frame sequence number
	unsigned int ns;
  // alarm
	unsigned int timeout;
	unsigned int numRetries;
	unsigned int alarmFlag;

	struct termios oldtio, newtio;

  Statistics stats;
} DataLink;

extern DataLink dataLink;

// Data link structure
int setDataLinkStruct(const char* port, int role);
int displayStatistics();

// aux ll functions
int startConnection();
int stopConnection(int fd);
int dataStateMachine(enum state* connection_state, unsigned char byte_read);
unsigned char calculateDataBCC(const unsigned char* dataBuffer, int length);
unsigned char communicationStateMachine(enum state* connection_state, unsigned char byte_read);

// ll functions
int llopen(const char* port, int role);
int llclose(int fd, int role);
int llwrite(int fd, unsigned char* packet, int length);
int llread(int fd, unsigned char* buf);

// Transmitter
int readResponse(int fd, const unsigned char expected[]);
int writeFrame(int fd, unsigned char* packet, int length);
int readAck(int fd);

// Receiver
int readCommand(int fd, const unsigned char expected[]);
int readFrame(int fd, unsigned char* buf);
int destuffFrame(unsigned char* frame, int frame_length, unsigned char* destuffedFrame);
int verifyDataPacketReceived(unsigned char * buffer, int size);
