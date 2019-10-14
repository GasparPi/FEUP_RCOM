#include "ll.h"

const unsigned char SET[] = {FLAG, A_CMD, C_SET, BCC(A_CMD, C_SET),FLAG};
const unsigned char UA[] = {FLAG, A_CMD, C_UA, BCC(A_CMD, C_UA), FLAG};
const unsigned char DISC[] = {FLAG, A_CMD, C_DISC, BCC(A_CMD, C_DISC), FLAG};
const unsigned char RR0[] = {FLAG, A_CMD, C_RR0, BCC(A_CMD, C_RR0), FLAG};
const unsigned char RR1[] = {FLAG, A_CMD, C_RR1, BCC(A_CMD, C_RR1), FLAG};
const unsigned char REJ0[] = {FLAG, A_CMD, C_REJ0, BCC(A_CMD, C_REJ0), FLAG};
const unsigned char REJ1[] = {FLAG, A_CMD, C_REJ1, BCC(A_CMD, C_REJ1), FLAG};

int alarmFlag = 1, numRetry = 0;
struct termios newtio, oldtio;

int llopen(const char* port, int role) {

	int fd, res;

	(void) signal(SIGALRM, alarmHandler);

	// Config termios struct
	fd = startConnection(port);
	if (role == TRANSMITTER) {
	
		do {
			printf("Sending SET Message!\n");
			// send SET message
			res = write(fd, SET, sizeof(SET)/sizeof(unsigned char));
			alarm(MAX_TIMEOUT); // install alarm
			alarmFlag = 0;
			// read UA frame
			readResponse(fd, UA);

  		} while(numRetry < MAX_RETRIES && alarmFlag);

		alarm(0); // uninstall alarm

  		if (numRetry >= MAX_RETRIES) {
    		printf("MAX RETRIES!!!\n");
    		return -1;
  		}

  		numRetry = 0;

	} else if (role == RECEIVER) {
		// read SET frame
    	if (readCommand(fd, SET) == -1)
			return -2;
		// send UA frame
		write(fd, UA, sizeof(UA)/sizeof(unsigned char));

	} else {
		printf("Unkown role: %d\n", role);
		return 1;
	}

	return fd;
}

int llclose(int fd, int role) {
	int res;

	if (role == TRANSMITTER) {
		do {
			printf("Sending DISC Message!\n");
			// send DISC frame
			res = write(fd, DISC, sizeof(DISC)/sizeof(unsigned char));
			alarm(MAX_TIMEOUT); // install alarm
			alarmFlag = 0;
			// read DISC frame
			printf("Reading DISC Message!\n");
			readResponse(fd, DISC);

  		} while(numRetry < MAX_RETRIES && alarmFlag);

		alarm(0); // uninstall alarm

  		if (numRetry >= MAX_RETRIES)
    		printf("MAX RETRIES!!!\n");
		else {
			printf("SENDING UA MESSAGE\n");
			// send UA frame		
			write(fd, UA, sizeof(UA)/sizeof(unsigned char));
		}

	} else if (role == RECEIVER) {
		// read DISC frame
		printf("Reading DISC\n");
    	if (readCommand(fd, DISC) == -1)
			return -2;
		// send DISC frame
		printf("Writing DISC\n");
		write(fd, DISC, sizeof(DISC)/sizeof(unsigned char));

		sleep(2);
		// read UA FRAME
		printf("Reading UA\n");
		if (readCommand(fd, UA) == -1)
			return -3;
	} else {
		printf("Unkown role: %d\n", role);
		return 1;
	}

	// close file descriptors and set old termios struct
	stopConnection(fd);

	return 0;
}

int stopConnection(int fd) {

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);

	return 0;
}

int startConnection(const char* port) {
	/*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  	*/
  	int fd = open(port, O_RDWR | O_NOCTTY );
  	if (fd < 0) {
		perror(port); 
		exit(-1); 
	}
	  	if (tcgetattr(fd, &oldtio) == -1) { // save current port settings
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
	newtio.c_cc[VMIN] = 0;   /* does no block read */

	/* 
	TIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
	leitura do(s) próximo(s) caracter(es)
	*/
	tcflush(fd, TCIOFLUSH);
	
	if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	return fd;
}


int readResponse(int fd, const unsigned char expected[]) {

 	unsigned char byte_read;
 	enum state current_state = START;
 	printf("Starting state machine\n");

 	while (!alarmFlag && current_state != STOP) {
        
	    read(fd, &byte_read, 1);

	    switch(current_state) {
		    case START: {
		        if(byte_read == expected[0])       
		        	current_state = FLAG_RCV;
		        break;
		    }
		    case FLAG_RCV: {
		        if(byte_read == expected[1])
		    	    current_state = A_RCV;
		        else if(byte_read != FLAG)
		        	current_state = START;
		        break;    
		    }
		    case A_RCV: {
		        if (byte_read == expected[2])
		        	current_state = C_RCV;
		        else if (byte_read == FLAG_RCV)
		        	current_state = FLAG_RCV;
		        else
		        	current_state = START; 
		        break;      
		    }
	      	case C_RCV: {
		        if (byte_read == expected[3])
		        	current_state = BCC_OK;
		        else if (byte_read == FLAG_RCV)
		         	current_state = FLAG_RCV;
		        else 
		         	current_state = START; 
		        break;        
	      	}
	      	case BCC_OK: {
		        if (byte_read == expected[4]) {
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

int readCommand(int fd, const unsigned char expected[]) {

	enum state current_state = START;
	unsigned char byte_read;

	printf("Starting state machine\n");
	while (current_state != STOP) {
			
		read(fd, &byte_read, 1);

		switch(current_state) {
			case START: {
				if(byte_read == expected[0]) 			
					current_state = FLAG_RCV;
				break;
			}
			case FLAG_RCV: {
				if(byte_read == expected[1])
					current_state = A_RCV;
				else if(byte_read != FLAG)
					current_state = START;
				break;		
			}
			case A_RCV: {
				if (byte_read == expected[2])
					current_state = C_RCV;
				else if (byte_read == FLAG_RCV)
					current_state = FLAG_RCV;
				else current_state = START;	
				break;			
			}
			case C_RCV: {
				if (byte_read == expected[3])
					current_state = BCC_OK;
				else if (byte_read == FLAG_RCV)
					current_state = FLAG_RCV;
				else current_state = START;	
				break;				
			}
			case BCC_OK: {
				if (byte_read == expected[4]) {
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

void alarmHandler() {
 	printf("Alarm: %d\n", numRetry + 1);
 	alarmFlag = 1;
 	numRetry++;
}

int llwrite(int fd, char* buf, int length) {

	char frame[255];
	int Ns = 0, dataFrameSize = 0, frameIndex, framesWritten = 0, bcdResult;
	char bufChar;
	
	while (length > 0) {
		frame[0] = FLAG;
		frame[1] = A_CMD;
		frame[2] = (Ns == 0 ? C0 : C1);
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

int llread(int fd, unsigned char* buf) {
	int received = 0;
	int bytesRead = 1;
	unsigned char packet[MAX_PACKET_SIZE]; 

	while(!received) {
		// readPacket
		if (readPacket(fd, &packet)) {
			// destuff packet
			// verify data packet
			// send response accordingly
		}
		received = 1;
	}

	return bytesRead;
}

int readPacket(int fd, unsigned char* buf[]) {
	enum state connection_state = START;
	int length = 0;
	unsigned char read_byte;

	while(connection_state != STOP) {
		read(fd, &read_byte, 1);

		dataStateMachine(&connection_state, read_byte);

		if(connection_state == FLAG_RCV && length != 0)
			length = 0;
		else if(connection_state == DATA_RCV)
			connection_state = BCC_OK;

		*buf[length] = read_byte;
		length++;
	}

	return length;
}

int dataStateMachine(enum state* connection_state, unsigned char read_byte) {
	switch(*connection_state) {
		case START:
			if(read_byte == FLAG){
				*connection_state = FLAG_RCV;
			}
			break;

		case FLAG_RCV:
			if(read_byte == A_CMD){
				*connection_state = A_RCV;
			}
			else if(read_byte == FLAG)
				break;
			else
				*connection_state = START;
			break;

		case A_RCV:
			if(read_byte == C0 || read_byte == C1)
				*connection_state = C_RCV;
			else if(read_byte == FLAG)
				*connection_state = FLAG_RCV;
			else
				*connection_state = START;
			break;

		case C_RCV:
			if(read_byte == BCC(A_CMD, C0) || read_byte == BCC(A_CMD, C1) )
				*connection_state = BCC_OK;
			else if(read_byte == FLAG)
				*connection_state = FLAG_RCV;
			else 
				*connection_state = START;
			break;

		case BCC_OK:
			if(read_byte == FLAG) //received command
				*connection_state = STOP;
			else if(read_byte != FLAG) //received data
				*connection_state = DATA_RCV;
			break;
		
		default:
			break;
	}

	return 0;
}

