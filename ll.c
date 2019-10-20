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

	int fd;

	// Config termios struct
	fd = startConnection(port);
	if (role == TRANSMITTER) {

		do {
			printf("Sending SET Message!\n");
			// send SET message
			write(fd, SET, sizeof(SET)/sizeof(unsigned char));
			setAlarm(); // install alarm
			alarmFlag = 0;
			// read UA frame
			readResponse(fd, UA);

  	} while(numRetry < MAX_RETRIES && alarmFlag);

		stopAlarm(); // uninstall alarm

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

	if (role == TRANSMITTER) {
		do {
			printf("Sending DISC Message!\n");
			// send DISC frame
			write(fd, DISC, sizeof(DISC)/sizeof(unsigned char));
			setAlarm(); // install alarm
			alarmFlag = 0;
			// read DISC frame
			printf("Reading DISC Message!\n");
			readResponse(fd, DISC);

  	} while(numRetry < MAX_RETRIES && alarmFlag);

		stopAlarm(); // uninstall alarm

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
	newtio.c_cc[VMIN] = 1;   /* blocks until 1 byte is read */

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

 	while (!alarmFlag && current_state != STOP) {
	    read(fd, &byte_read, 1);
	    communicationStateMachine(&current_state, byte_read);
  	}

 	return 0;
}

int readAck(int fd, int Ns) {
 	unsigned char byte_read, control_field;
 	enum state current_state = START;

	printf("Starting state machine\n");

 	while (!alarmFlag && current_state != STOP) {
	  read(fd, &byte_read, 1);
		control_field = communicationStateMachine(&current_state, byte_read);
  }

	if (control_field == C_RR0 && Ns == 1)
		return 0;
	else if (control_field == C_RR1 && Ns == 0)
		return 0;
	else if (control_field == C_REJ0 && Ns == 1)
		return -1;
	else if (control_field == C_REJ1 && Ns == 0)
		return -1;
	else
		return -1; // undefined error
}

unsigned char communicationStateMachine(enum state* connection_state, unsigned char byte_read) {
	static unsigned char control_field = 0;

	switch(*connection_state) {
		case START: {
			if(byte_read == FLAG)
				*connection_state = FLAG_RCV;
			break;
		}
		case FLAG_RCV: {
			if(byte_read == A_CMD)
				*connection_state = A_RCV;
			else if(byte_read != FLAG)
				*connection_state = START;
			break;
		}
		case A_RCV: {
			if (CHECK_C(byte_read)) {
				*connection_state = C_RCV;
				control_field = byte_read;
			}
			else if (byte_read == FLAG_RCV)
				*connection_state = FLAG_RCV;
			else
				*connection_state = START;
			break;
		}
		case C_RCV: {
			if (byte_read == (BCC(A_CMD, control_field)))
				*connection_state = BCC_OK;
			else if (byte_read == FLAG_RCV)
				*connection_state = FLAG_RCV;
			else
				*connection_state = START;
			break;
		}
		case BCC_OK: {
			if (byte_read == FLAG) {
				*connection_state = STOP;
			}
			else
				*connection_state = START;
			break;
		}
		case DATA_RCV:
			break;
		case STOP:
			break;
	};

	return control_field;
}

int readCommand(int fd, const unsigned char expected[]) {
	enum state current_state = START;
	unsigned char byte_read;

	while (current_state != STOP) {
		read(fd, &byte_read, 1);
		communicationStateMachine(&current_state, byte_read);
	}

	return 0;
}

int llwrite(int fd, char* packet, int length) {
	int Ns = 0;
	int bytesWritten = 0;
	do {
		printf("Sending frame!\n");
		// send frame
		bytesWritten = writeFrame(fd, packet, length, Ns);
		setAlarm(); // install alarm
		alarmFlag = 0;
		// read receiver response
		if (readAck(fd, Ns) == -1) {
			stopAlarm();
			continue;
		}
	} while(numRetry < MAX_RETRIES && alarmFlag);

	stopAlarm();

	if (numRetry == MAX_RETRIES)
		return -1;

	return bytesWritten;
}

int writeFrame(int fd, char* packet, int length, int Ns) {

	char frame[2 * length + 6]; // TODO clean 2 * ____ + 6
	int dataIndex, frameIndex, frameSize, bccResult;
	char packetChar;

	// Set of frame header
	frame[0] = FLAG;
	frame[1] = A_CMD;
	frame[2] = (Ns == 0 ? C0 : C1);
	frame[3] = BCC(A_CMD, frame[2]);

	//Reading first 2 chars to set bcd
	frame[4] = packet[0];
	frame[5] = packet[1];
	bccResult = frame[4] ^ frame[5];

	dataIndex = 2; //0 + 2
	frameIndex = 6; //4 + 2
	frameSize = 6;

	//Process data camp
	while (dataIndex < length) {

		packetChar = packet[dataIndex++];
		bccResult ^= packetChar;

		// Byte stuffing
		if (packetChar == FLAG || packetChar == ESC) {
			frame[frameIndex++] = ESC;
			frame[frameIndex++] = packetChar ^ STUFFING;
			frameSize += 2;
		}
		else {
			frame[frameIndex++] = packetChar;
			frameSize++;
		}
	}

	// Set of frame footer
	frame[frameIndex] = bccResult;
	frame[frameIndex + 1] = FLAG;
	frameSize += 2;
	write(fd, frame, frameSize);

	// printf("Sent frame size: %d\n", frameSize);
	return frameSize;
}

int llread(int fd, unsigned char* buf) {
	printf("INSIDE LLREAD\n");
	int received = 0;
	int bytesRead = 1;
	int frame_length = 0;
	unsigned char frame[MAX_FRAME_SIZE];
	unsigned char control_field;

	while(!received) {
		if ((frame_length = readFrame(fd, frame))) {
			// remove frame header and tail
			// destuff packet
			int j = 0;
			for (int i = 4; i < frame_length - 2; i++) { // TODO clean i = 4 and -2
				if (frame[i] == ESC) {
					i++;
					if (frame[i] == (FLAG ^ STUFFING))
						buf[j++] = FLAG;
					else if (frame[i] == (ESC ^ STUFFING))
						buf[j++] = ESC;
				}
				else {
					buf[j++] = frame[i];
				}
			}
			control_field = frame[2];

			printf("frame[4]: %x\npacket[0]: %x\n",frame[4] & 0xff, buf[0] & 0xff);

			// verify data packet
			if(verifyDataPacketReceived(frame, frame_length) != 0) {
				// send response accordingly
				printf("LLREAD: Packet is not data or has header errors\n"); //does not save packet
				if (control_field == C0)
					write(fd, REJ1, sizeof(REJ1)/sizeof(unsigned char));
				else if (control_field == C1)
					write(fd, REJ0, sizeof(REJ0)/sizeof(unsigned char));
			}
			else {
				if (control_field == C0)
					write(fd, RR1, sizeof(RR1)/sizeof(unsigned char));
				else if (control_field == C1)
					write(fd, RR0, sizeof(RR0)/sizeof(unsigned char));
				received = 1;
			}
		}
	}

	printf("END OF LLREAD\n");

	return bytesRead;
}

unsigned char calculateDataBCC(const unsigned char* dataBuffer, int length) {
	unsigned char bcc = 0x00;

	for (int i = 0; i < length; i++)
		bcc ^= dataBuffer[i];

	return bcc;
}

int verifyDataPacketReceived(unsigned char* buffer, int size){
	unsigned char address_field = buffer[1];
	unsigned char control_field = buffer[2];
	unsigned char headerBCC = buffer[3];

	if(headerBCC == (BCC(address_field, control_field)) && (control_field == C0 || control_field == C1)){
		unsigned char calculatedDataBCC = calculateDataBCC(&buffer[4], size-6); // between buffer[4] and buffer[size-(4+2)] -> DATA
		unsigned char dataBCC = buffer[size - 2];

		if(dataBCC != calculatedDataBCC) {
			printf("LLREAD: Bad dataBCC\n");
			return -2;
		}
	}
	else if(control_field != C0 && control_field != C1){
		printf("LLREAD: Bad control field: %d\n", control_field);
		return -1;
	}

	return 0;
}

int readFrame(int fd, unsigned char* buf) {
	enum state connection_state = START;
	int length = 0;
	unsigned char byte_read;

	while(connection_state != STOP) {

		read(fd, &byte_read, 1);

		dataStateMachine(&connection_state, byte_read);

		if(connection_state == FLAG_RCV && length != 0)
			length = 0;

		buf[length++] = byte_read;
	}

	return length;
}

int dataStateMachine(enum state* connection_state, unsigned char byte_read) {
	switch(*connection_state) {
		case START:
			if(byte_read == FLAG){
				*connection_state = FLAG_RCV;
			}
			break;
		case FLAG_RCV:
			if(byte_read == A_CMD){
				*connection_state = A_RCV;
			}
			else if(byte_read == FLAG)
				break;
			else
				*connection_state = START;
			break;
		case A_RCV:
			if(byte_read == C0 || byte_read == C1)
				*connection_state = C_RCV;
			else if(byte_read == FLAG)
				*connection_state = FLAG_RCV;
			else
				*connection_state = START;
			break;
		case C_RCV:
			if(byte_read == (BCC(A_CMD, C0)) || byte_read == (BCC(A_CMD, C1)) )
				*connection_state = BCC_OK;
			else if(byte_read == FLAG)
				*connection_state = FLAG_RCV;
			else
				*connection_state = START;
			break;
		case BCC_OK:
			if(byte_read != FLAG) //received data
				*connection_state = DATA_RCV;
			break;

		case DATA_RCV:
			if (byte_read == FLAG)
				*connection_state = STOP;

		case STOP:
			break;

		default:
			break;
	}

	return 0;
}
