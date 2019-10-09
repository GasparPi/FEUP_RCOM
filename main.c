/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>

#include "ll.h"

#define TRANSMITTER 0
#define RECEIVER 1

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int main(int argc, char** argv) {

  int stop_alarm = 0;
    
  if ( (argc < 3) || ((strcmp("/dev/ttyS0", argv[1]) != 0 ) && (strcmp("/dev/ttyS4", argv[1]) != 0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS4\n");
    exit(1);
  }

  /*
  *  Opens Serial Port communication 
  */
  if (llopen(argv[1], argv[2]) == -1) {
    perror("LLOPEN");
    return -1;
  }

	
  tcflush(fd, TCIOFLUSH);
     
  if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);

  return 0;
}


