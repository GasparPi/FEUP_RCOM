#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>

#define FLAG 0x7E
#define A_CMD 0x03
#define A_REP 0x01 // Not used yet :)
#define C_SET 0x03
#define C_UA 0x07
#define BCC(X, Y) (X) ^ (Y)
#define MAX_RETRIES 3
#define MAX_TIMEOUT 3

#define TRANSMITTER 0
#define RECEIVER 1

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

// aux functions
int startConnection(const char* port);
int stopConnection(int fd);

// ll functions
int llopen(const char* port, int role);
int llclose(int fd);


// Transmitter
int readResponse(int fd);
void alarmHandler();
int sendSet(int fd, unsigned char *frame);

// Receiver
int readCommand(int fd);

enum state {
  START,
  FLAG_RCV,
  A_RCV,
  C_RCV,
  BCC_OK,
  STOP
};


