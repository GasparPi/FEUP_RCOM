/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define C_SET 0x03
#define C_UA 0x07
#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define BCC_UA A ^ C_UA
#define BCC_SET A ^ C_SET

enum state
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
	STOP
};

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char frame[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

	sleep(1);

 	if(readMessage(fd) == -1)
		return -1;
 	else {
		frame[0] = FLAG;
		frame[1] = A;
		frame[2] = C_UA;
		frame[3] = BCC_UA;
		frame[4] = FLAG;
	
		write(fd, frame, sizeof(frame)/sizeof(unsigned char));
	}

	sleep(1);
	tcflush(fd, TCIOFLUSH);
	
    tcsetattr(fd,TCSANOW,&oldtio);
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
				if(byte_read == A)
					current_state = A_RCV;
				else if(byte_read != FLAG)
					current_state = START;
				break;		
			}
			case A_RCV: {
				printf("IN a_rcv state!\n");
				if (byte_read == C_SET)
					current_state = C_RCV;
				else if (byte_read == FLAG_RCV)
					current_state = FLAG_RCV;
				else current_state = START;	
				break;			
			}
			case C_RCV: {
				printf("IN c_rcv state!\n");
				unsigned char bcc = BCC_SET;
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


