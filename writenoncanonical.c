/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

// TP2
#define FLAG 0x7E
#define A_CMD 0x03
#define A_REP 0x01 // Not used yet :)
#define C_SET 0x03
#define C_UA 0x07
#define BCC(A, C) (A) ^ (C)

#define MAX_RETRIES 3

int alarmFlag=1, numRetry=0;

enum state
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
};

int main(int argc, char** argv) {

  int fd, c, res;
  struct termios oldtio,newtio;

  unsigned char frame[255];
    
  if ( (argc < 2) || ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS4", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS4\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  (void) signal(SIGALRM, alarmHandler());

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0;  /* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;   /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */
  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  while (numRetry < MAX_RETRIES) {
    res = sendSet(fd, frame);
    alarm(MAX_RETRIES);
    alarmFlag = 0

    while (alarmFlag) {
      printf("cenas\n");
      readMessage()
    }
  } 

  sleep(1);

  if(readMessage(fd) == -1)
    return -1;

  tcflush(fd, TCIOFLUSH);
  sleep(1);
   
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);

  return 0;
}

int readMessage(int fd){
  
  enum state current_state = START;

  unsigned char byte_read;
  printf("Starting state machine\n");
  while (current_state != STOP) {
  
    if (read(fd, &byte_read, 1) != 1) return -1;
    switch(current_state){
      case START: {
        printf("In START state\n");
        if(byte_read == FLAG)       
          current_state = FLAG_RCV;
        break;
      }
      case FLAG_RCV: {
        printf("IN flag_rcv state!\n");
        if(byte_read == A_CMD)
          current_state = A_RCV;
        else if(byte_read != FLAG)
          current_state = START;
        break;    
      }
      case A_RCV: {
        printf("IN a_rcv state!\n");
        if (byte_read == C_UA)
          current_state = C_RCV;
        else if (byte_read == FLAG_RCV)
          current_state = FLAG_RCV;
        else current_state = START; 
        break;      
      }
      case C_RCV: {
        printf("IN c_rcv state!\n");
        unsigned char bcc = BCC(A_CMD, C_UA);
        if (byte_read == bcc)
          current_state = BCC_OK;
        else if (byte_read == FLAG_RCV)
          current_state = FLAG_RCV;
        else current_state = START; 
        break;        
      }
      case BCC_OK: {
        printf("IN bcc_ok state!\n");
        if (byte_read == FLAG)
          current_state = STOP;
        else current_state = START;
        break;      
      }
    };
  }

  printf("Everything OK!\n");

  return 0;
}

int sendSet(int fd, unsigned char *frame){
  // building frame
  frame[0] = FLAG;
  frame[1] = A_CMD;
  frame[2] = C_SET; // we want to send a set command
  frame[3] = BCC(A_CMD, C_SET); // we want to send a set command
  frame[4] = FLAG;

  // gets(buf); we used to get "ola" from the keyboard
  return write(fd, frame, sizeof(frame)/sizeof(unsigned char));

}

void alarmHandler() {
  printf("alarme # %d\n", numRetry);
  alarmFlag=1;
  numRetry++;
}
