#include "ll.h"

int alarmFlag = 1, numRetry = 0;
struct termios newtio, oldtio;

int stopConnection(int fd) {
	if (tcsetattr(fd,TCSANOW, &oldtio)== -1) {
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);
}

int startConnection(const char* port) {
	int fd;
	/*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  	*/
  	fd = open(port, O_RDWR | O_NOCTTY );
  	if (fd < 0) {
		perror(port); 
		exit(-1); 
	}

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
	newtio.c_cc[VMIN] = 0;   /* blocking read until 5 chars received */

	/* 
	TIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
	leitura do(s) próximo(s) caracter(es)
	*/
	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	return fd;
}

int llopen(const char* port, int role) {

	unsigned char frame[255];
	int fd, res;

	(void) signal(SIGALRM, alarmHandler);

	// Config termios struct
	fd = startConnection(port);

	if (role == TRANSMITTER) {
	
		do {
			printf("Sending SET Message!\n");
			res = sendSet(fd, frame);
			alarm(MAX_TIMEOUT);
			alarmFlag = 0;

			readResponse(fd);

  		} while(numRetry < MAX_RETRIES && alarmFlag);

  		if (numRetry >= MAX_RETRIES) {
    		printf("MAX RETRIES!!!\n");
    		return -1;
  		}

  		return 0;

	} else if (role == RECEIVER) {

    	if (readCommand(fd) == -1) {
			printf("AQUI!\n");
			return -1;
		}
 		else {
			frame[0] = FLAG;
			frame[1] = A_CMD;
			frame[2] = C_UA;
			frame[3] = BCC(A_CMD, C_UA);
			frame[4] = FLAG;
	
			write(fd, frame, sizeof(frame)/sizeof(unsigned char));
		}

	} else {
		printf("Unkown role: %d\n", role);
		return 1;
	}

	return fd;
}

int llclose(int fd) {
	// envia DISC recebe UA


	stopConnection(fd);

	return 0;
}



int readResponse(int fd) {
  
  unsigned char byte_read;
  enum state current_state = START;
  printf("Starting state machine\n");

  while (!alarmFlag && current_state != STOP) {
        
    read(fd, &byte_read, 1);

    switch(current_state){
      case START: {
        if(byte_read == FLAG)       
          current_state = FLAG_RCV;
        break;
      }
      case FLAG_RCV: {
        if(byte_read == A_CMD)
          current_state = A_RCV;
        else if(byte_read != FLAG)
          current_state = START;
        break;    
      }
      case A_RCV: {
        if (byte_read == C_UA)
          current_state = C_RCV;
        else if (byte_read == FLAG_RCV)
          current_state = FLAG_RCV;
        else
          current_state = START; 
        break;      
      }
      case C_RCV: {
        unsigned char bcc = BCC(A_CMD, C_UA);
        if (byte_read == bcc)
          current_state = BCC_OK;
        else if (byte_read == FLAG_RCV)
          current_state = FLAG_RCV;
        else 
          current_state = START; 
        break;        
      }
      case BCC_OK: {
        if (byte_read == FLAG) {
          current_state = STOP;
          printf("Everything OK!\n");          
        }
        else 
          current_state = START;
        break;      
      }
    };
  }


  return 0;
}

int sendSet(int fd, unsigned char *frame) {
  // building frame
  frame[0] = FLAG;
  frame[1] = A_CMD;
  frame[2] = C_SET; // we want to send a set command
  frame[3] = BCC(A_CMD, C_SET); // we want to send a set command
  frame[4] = FLAG;

  return write(fd, frame, sizeof(frame)/sizeof(unsigned char));
}

void alarmHandler() {
  printf("Alarm: %d\n", numRetry + 1);
  alarmFlag = 1;
  numRetry++;
}

int readCommand(int fd) {
	
	enum state current_state = START;
	unsigned char byte_read;

	printf("Starting state machine\n");
	while (current_state != STOP) {
			
		read(fd, &byte_read, 1);
		switch(current_state){
			case START: {
				if(byte_read == FLAG) 			
					current_state = FLAG_RCV;
				break;
			}
			case FLAG_RCV: {
				if(byte_read == A_CMD)
					current_state = A_RCV;
				else if(byte_read != FLAG)
					current_state = START;
				break;		
			}
			case A_RCV: {
				if (byte_read == C_SET)
					current_state = C_RCV;
				else if (byte_read == FLAG_RCV)
					current_state = FLAG_RCV;
				else current_state = START;	
				break;			
			}
			case C_RCV: {
				if (byte_read == BCC(A_CMD, C_SET))
					current_state = BCC_OK;
				else if (byte_read == FLAG_RCV)
					current_state = FLAG_RCV;
				else current_state = START;	
				break;				
			}
			case BCC_OK: {
				if (byte_read == FLAG) {
					printf("Everything OK!\n");
					current_state = STOP;
				}
				else
					current_state = START;
				break;			
			}
		};
	}

	return 0;
}

int llwrite(int fd, char* buf, int length) {

	char frame[255];
	int Ns = 0, dataFrameSize = 0, frameIndex, framesWritten = 0, bcdResult;
	char bufChar;
	
	while (length > 0) {
		frame[0] = FLAG;
		frame[1] = A_CMD;
		frame[2] = (Ns == 0 ? N0 : N1);
		frame[3] = BCC(A_CMD, frame[2]);

		//Reading first 2 chars to set bcd
		frame[4] = buf[0];
		frame[5] = buf[1];
		bcdResult = frame[4] ^ frame[5];
 		
		dataFrameSize = 2; //0 + 2
		frameIndex = 6; //4 + 2

		//Process data camp
		while (dataFrameSize < MAX_FRAME_SIZE && dataFrameSize < length) {

			bufChar = buf[dataFrameSize + framesWritten * MAX_FRAME_SIZE];
			bcdResult ^= bufChar;
			frame[frameIndex++] = bufChar;
			dataFrameSize++;
		}
		
		frame[dataFrameSize] = bcdResult;
		frame[dataFrameSize + 1] = FLAG;
		
		write(fd, frame, sizeof(frame)/sizeof(unsigned char));
		framesWritten++;
	}
}



