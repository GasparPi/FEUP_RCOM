#include "ll.h"

const unsigned char SET[] = {FLAG, A_CMD, C_SET, BCC(A_CMD, C_SET),FLAG};
const unsigned char UA[] = {FLAG, A_CMD, C_UA, BCC(A_CMD, C_UA), FLAG};
const unsigned char DISC[] = {FLAG, A_CMD, C_DISC, BCC(A_CMD, C_DISC), FLAG};
const unsigned char RR0[] = {FLAG, A_CMD, C_RR0, BCC(A_CMD, C_RR0), FLAG};
const unsigned char RR1[] = {FLAG, A_CMD, C_RR1, BCC(A_CMD, C_RR1), FLAG};
const unsigned char REJ0[] = {FLAG, A_CMD, C_REJ0, BCC(A_CMD, C_REJ0), FLAG};
const unsigned char REJ1[] = {FLAG, A_CMD, C_REJ1, BCC(A_CMD, C_REJ1), FLAG};

DataLink dataLink;

int setDataLinkStruct(const char* port, int role) {
	strcpy(dataLink.port, port);
	dataLink.mode = role;
	dataLink.baudrate = BAUDRATE;
	dataLink.ns = 1;
	dataLink.timeout = MAX_TIMEOUT;
	dataLink.alarmFlag = 1;
	dataLink.numRetries = 0;

	Statistics statistics;
	statistics.numSentIFrames = 0;
	statistics.numReceivedIFrames = 0;
	statistics.timeouts = 0;
	statistics.numSentRR = 0;
	statistics.numReceivedRR = 0;
	statistics.numSentREJ = 0;
	statistics.numReceivedREJ = 0;
	statistics.start = clock();

	dataLink.stats = statistics;

	return 0;
}

int llopen(const char* port, int role) {

	int fd;
	setDataLinkStruct(port, role);

	// Config termios struct
	fd = startConnection();
	if (dataLink.mode == TRANSMITTER) {

		do {
			printf("Writing SET\n");
			// send SET message
			write(fd, SET, sizeof(SET)/sizeof(unsigned char));
			setAlarm(); // install alarm
			dataLink.alarmFlag = 0;
			// read UA frame
			readResponse(fd, UA);

  	} while(dataLink.numRetries < MAX_RETRIES && dataLink.alarmFlag);

		stopAlarm(); // uninstall alarm

  	if (dataLink.numRetries >= MAX_RETRIES) {
    	printf("MAX RETRIES!!!\n");
    	return -1;
  	}

  	dataLink.numRetries = 0;

	} else if (dataLink.mode == RECEIVER) {
		// read SET frame
    	if (readCommand(fd, SET) == -1)
			return -2;
		// send UA frame
		write(fd, UA, sizeof(UA)/sizeof(unsigned char));

	} else {
		printf("Unkown role: %d\n", dataLink.mode);
		return 1;
	}


	return fd;
}

int llclose(int fd, int role) {

	if (dataLink.mode == TRANSMITTER) {
		do {
			printf("Writing DISC\n");
			// send DISC frame
			write(fd, DISC, sizeof(DISC)/sizeof(unsigned char));
			setAlarm(); // install alarm
			dataLink.alarmFlag = 0;
			// read DISC frame
			printf("Reading DISC\n");
			readResponse(fd, DISC);
  	} while(dataLink.numRetries < MAX_RETRIES && dataLink.alarmFlag);

		stopAlarm(); // uninstall alarm

  	if (dataLink.numRetries >= MAX_RETRIES)
    		printf("MAX RETRIES!!!\n");
		else {
			printf("Writing UA\n");
			// send UA frame
			write(fd, UA, sizeof(UA)/sizeof(unsigned char));
			sleep(1); // "wait" for receiver to read file UA
		}
	} else if (dataLink.mode == RECEIVER) {

		printf("***Closing connection***\n\n");

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
		printf("Unkown role: %d\n", dataLink.mode);
		return 1;
	}

	// close file descriptors and set old termios struct
	stopConnection(fd);

	return 0;
}

int stopConnection(int fd) {

	dataLink.stats.end = clock();

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &dataLink.oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);

	printf("\nDATA TRANFER CONCLUDED SUCCESFULLY\n\n");

	return 0;
}

int startConnection() {
	/*
	Open serial port device for reading and writing and not as controlling tty
	because we don't want to get killed if linenoise sends CTRL-C.
	*/


	int fd = open(dataLink.port, O_RDWR | O_NOCTTY );
	if (fd < 0) {
			perror(dataLink.port);
			exit(-1);
		}
		if (tcgetattr(fd, &dataLink.oldtio) == -1) { // save current port settings
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&dataLink.newtio, sizeof(dataLink.newtio));
	dataLink.newtio.c_cflag = dataLink.baudrate | CS8 | CLOCAL | CREAD;
	dataLink.newtio.c_iflag = IGNPAR;
	dataLink.newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	dataLink.newtio.c_lflag = 0;

	dataLink.newtio.c_cc[VTIME] = 0;  /* inter-character timer unused */
	dataLink.newtio.c_cc[VMIN] = 1;   /* blocks until 1 byte is read */

	/*
	TIME e VMIN devem ser alterados de forma a proteger com um temporizador a
	leitura do(s) próximo(s) caracter(es)
	*/
	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &dataLink.newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	return fd;
}


int readResponse(int fd, const unsigned char expected[]) {
 	unsigned char byte_read;
 	enum state current_state = START;

 	while (!dataLink.alarmFlag && current_state != STOP) {
	    read(fd, &byte_read, 1);
	    communicationStateMachine(&current_state, byte_read);
  }

 	return 0;
}

int readAck(int fd) {
 	unsigned char byte_read, control_field;
 	enum state current_state = START;

 	while (!dataLink.alarmFlag && current_state != STOP) {
		read(fd, &byte_read, 1);
		control_field = communicationStateMachine(&current_state, byte_read);
  }

	if (control_field == C_RR0 && dataLink.ns == 1) {
		printf("RR received: %d\n", dataLink.ns);
		dataLink.stats.numReceivedRR++;
		return 0;
	}
	else if (control_field == C_RR1 && dataLink.ns == 0) {
		printf("RR received: %d\n", dataLink.ns);
		dataLink.stats.numReceivedRR++;
		return 0;
	}
	else if (control_field == C_REJ0 && dataLink.ns == 1) {
		printf("REJ received: %d\n", dataLink.ns);
		dataLink.stats.numReceivedREJ++;
		return -1;
	}
	else if (control_field == C_REJ1 && dataLink.ns == 0) {
		printf("REJ received: %d\n", dataLink.ns);
		dataLink.stats.numReceivedREJ++;
		return -1;
	}
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

		default:
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

int llwrite(int fd, unsigned char* packet, int length) {
	dataLink.ns = (dataLink.ns == 0);

	do {
		// send frame
		writeFrame(fd, packet, length);
		setAlarm(); // install alarm
		dataLink.alarmFlag = 0;
		// read receiver response
		if (readAck(fd) == -1) { // REJ received
			stopAlarm();
			dataLink.alarmFlag = 1;
			continue;
		}
	} while(dataLink.numRetries < MAX_RETRIES && dataLink.alarmFlag);

	stopAlarm();

	if (dataLink.numRetries == MAX_RETRIES)
		return -1;

	dataLink.numRetries = 0;

	return length;
}

int writeFrame(int fd, unsigned char* packet, int length) {

	unsigned char frame[2 * length + 6]; // TODO clean 2 * ____ + 6
	int dataIndex, frameIndex;
	unsigned char packetChar;

	// Set of frame header
	frame[0] = FLAG;
	frame[1] = A_CMD;
	frame[2] = (dataLink.ns == 0 ? C0 : C1);
	frame[3] = BCC(A_CMD, frame[2]);

	unsigned char bccResult = calculateDataBCC(packet, length);

	dataIndex = 0;
	frameIndex = 4;

	//Process data camp
	while (dataIndex < length) {

		packetChar = packet[dataIndex++];

		// Byte stuffing
		if (packetChar == FLAG || packetChar == ESC) {
			frame[frameIndex++] = ESC;
			frame[frameIndex++] = packetChar ^ STUFFING;
		}
		else {
			frame[frameIndex++] = packetChar;
		}
	}

	// Set of frame footer
	if (bccResult == ESC || bccResult == FLAG) {
		frame[frameIndex++] = ESC;
		frame[frameIndex++] = bccResult ^ STUFFING;
	}
	else {
		frame[frameIndex++] = bccResult;
	}

	frame[frameIndex++] = FLAG;
	write(fd, frame, frameIndex);
	dataLink.stats.numSentIFrames++;

	printf("Sent frame size: %d\n", frameIndex);
	return frameIndex;
}

int llread(int fd, unsigned char* buf) {
	int received = 0;
	int frame_length = 0;
	int destuffed_frame_length = 0;
	int packet_length = 0;
	unsigned char frame[MAX_FRAME_SIZE];
	unsigned char destuffedFrame[MAX_FRAME_SIZE];
	unsigned char control_field;

	while(!received) {
		if ((frame_length = readFrame(fd, frame))) {
			// destuff packet
			destuffed_frame_length = destuffFrame(frame, frame_length, destuffedFrame);
			control_field = frame[2];

			// verify data packet
			if(verifyDataPacketReceived(destuffedFrame, destuffed_frame_length) != 0) {
				// send response accordingly
				printf("LLREAD: Packet is not data or has header errors\n\n"); //does not save packet
				if (control_field == C0){
					write(fd, REJ1, sizeof(REJ1)/sizeof(unsigned char));
					dataLink.stats.numSentREJ++;
					printf("REJ sent: 1\n");
				}
				else if (control_field == C1){
					write(fd, REJ0, sizeof(REJ0)/sizeof(unsigned char));
					dataLink.stats.numSentREJ++;
					printf("REJ sent: 0\n");
				}

				return 0;
			}
			else {
				// save packet of buffer
				for (int i = 4; i < destuffed_frame_length - 2; i++) {
					buf[packet_length] = destuffedFrame[i];
					packet_length++;
				}

				if (control_field == C0){
					write(fd, RR1, sizeof(RR1)/sizeof(unsigned char));
					dataLink.stats.numSentRR++;
					printf("RR sent: 1\n");
				}
				else if (control_field == C1){
					write(fd, RR0, sizeof(RR0)/sizeof(unsigned char));
					dataLink.stats.numSentRR++;
					printf("RR sent: 0\n");
				}
				received = 1;
			}
		}
	}

	// of frame_length - not sure
	return packet_length;
}

int destuffFrame(unsigned char* frame, int frame_length, unsigned char* destuffedFrame) {
	// HEADER
	destuffedFrame[0] = frame[0]; // FLAG
	destuffedFrame[1] = frame[1]; // A
	destuffedFrame[2] = frame[2]; // C
	destuffedFrame[3] = frame[3]; // BCC1
	// DATA
	int j = 4;
	int i;
	for (i = 4; i < frame_length - 1; i++) {
		if (frame[i] == ESC) {
			i++;
			if (frame[i] == (FLAG ^ STUFFING))
				destuffedFrame[j++] = FLAG;
			else if (frame[i] == (ESC ^ STUFFING))
				destuffedFrame[j++] = ESC;
		}
		else {
			destuffedFrame[j++] = frame[i];
		}
	}

	// FOOTER
	destuffedFrame[j++] = frame[i++]; // FLAG

	return j;
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
			printf("\nLLREAD: Bad dataBCC\n");
			return -2;
		}
	}
	else if(control_field != C0 && control_field != C1){
		printf("\nLLREAD: Bad control field: %d\n", control_field);
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

	printf("Read frame length: %d\n", length);
	dataLink.stats.numReceivedIFrames++;
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

int displayStatistics() {
	printf("***Statistics:***\n\n");
	printf("Total execution time: %f seconds\n", (float) (dataLink.stats.end - dataLink.stats.start) / CLOCKS_PER_SEC);
	printf("Number of sent I frames: %d\n", dataLink.stats.numSentIFrames);
	printf("Number of received I frames: %d\n", dataLink.stats.numReceivedIFrames);
	printf("Number of timeouts: %d\n", dataLink.stats.timeouts);
	printf("Number of sent RR frames: %d\n", dataLink.stats.numSentRR);
	printf("Number of received RR frames: %d\n", dataLink.stats.numReceivedRR);
	printf("Number of sent REJ frames: %d\n", dataLink.stats.numSentREJ);
	printf("Number of received REJ frames: %d\n", dataLink.stats.numReceivedREJ);
	printf("\n");
	return 0;
}
