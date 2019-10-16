#include "ll.h"
#include <stdio.h>

int main(int argc, char** argv) {

	int stop_alarm = 0;
	int fd;

	if ( (argc < 4) || ((strcmp("/dev/ttyS0", argv[1]) != 0 ) && (strcmp("/dev/ttyS4", argv[1]) != 0) )) {
		printf("Usage:\t<SerialPort>\t<Sender == 0 || Receiver == 1>\t<Filename>\n");
		exit(1);
	}

	if (atoi(argv[2]) == 0){ //SENDER
		FILE *fptr;

		fptr = fopen(argv[3],"r");
		if(fptr == NULL){
		  printf("Error opening file!");
		  return -1;
		}
	}

	/*
	*  Opens Serial Port communication
	*/
	if ((fd = llopen(argv[1], atoi(argv[2]))) == -1) {
		perror("LLOPEN");
		return -1;
	}

	if (atoi(argv[2]) == 0){ //SENDER
		/*if(sendFile() == -1){
			printf("Error sending file.");
			return -1;
		}*/

	} else if (atoi(argv[2]) == 1){ //RECEIVER
		/*if(receiveFile() == -1){
			printf("Error sending file.");
			return -1;
		}*/
	}

	if (llclose(fd, atoi(argv[2])) == -1) {
		perror("LLCLOSE");
		return -1;
	}

	return 0;
}
