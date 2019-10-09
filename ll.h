#define FLAG 0x7E
#define A_CMD 0x03
#define A_REP 0x01 // Not used yet :)
#define C_SET 0x03
#define C_UA 0x07
#define BCC(X, Y) (X) ^ (Y)
#define MAX_RETRIES 3
#define MAX_TIMEOUT 3

int llopen(char* port, int role);

//Transmitter
int readResponse(int fd);
void alarmHandler();
int sendSet(int fd, unsigned char *frame);

//Receiver
int readCommand(int fd);

enum state {
  START,
  FLAG_RCV,
  A_RCV,
  C_RCV,
  BCC_OK,
  STOP
};


