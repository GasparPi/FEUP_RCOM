/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

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
#define BCC(A, C) (A) ^ (C)

volatile int STOP=FALSE;

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

  
  // building frame
  frame[0] = FLAG;
  frame[1] = A_CMD;
  frame[2] = C_SET; // we want to send a set command
  frame[3] = BCC(A_CMD, C_SET); // we want to send a set command
  frame[4] = FLAG;

  // gets(buf); we used to get "ola" from the keyboard
  res = write(fd, frame, sizeof(frame)/sizeof(unsigned char));
  
  printf("%d bytes written\n", res);

  sleep(1);

  tcflush(fd, TCIOFLUSH);

  exit(0);

  char buf_res[255];
  for(int i = 0; STOP==FALSE; i++) {
    char c;
    res = read(fd,&c,1);  
    buf_res[i] = c;       /* loop for input */
    if (c=='\0') STOP=TRUE;
  }

  printf("String read from serial port: %s\n", buf_res);

  sleep(1);
   
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);

  return 0;
}
