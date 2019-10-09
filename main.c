#include "ll.h"

int main(int argc, char** argv) {

  int stop_alarm = 0;
  int fd;
    
  if ( (argc < 3) || ((strcmp("/dev/ttyS0", argv[1]) != 0 ) && (strcmp("/dev/ttyS4", argv[1]) != 0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS4\n");
    exit(1);
  }

  /*
  *  Opens Serial Port communication 
  */
  if ((fd = llopen(argv[1], argv[2])) == -1) {
    perror("LLOPEN");
    return -1;
  }

  if (llclose(fd) == -1) {
    perror("LLCLOSE");
    return -1;
  }

  return 0;
}


