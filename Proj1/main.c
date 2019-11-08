#include "app.h"
#include <stdio.h>

int main(int argc, char** argv) {

	int fd;
	int fd_file;

	if ( (strcmp("/dev/ttyS0", argv[1]) == 0 ) || (strcmp("/dev/ttyS4", argv[1]) == 0) ) {
		if((atoi(argv[2]) == 0 && argc != 4) || (atoi(argv[2]) == 1 && argc != 3) || (atoi(argv[2]) != 1 && atoi(argv[2]) != 0)){
			printf("Usage:\t<SerialPort>\t<Sender == 0 || Receiver == 1>\t<Filename>\n");
			exit(1);
		}
	} else {
		printf("Please use ports /dev/ttyS0 or /dev/ttyS4\n");
		exit(1);
	}


	if (atoi(argv[2]) == 0){ //GET FILE'S FILE DESCRIPTOR

		printf("File name: %s\n", argv[3]);

		fd_file = open(argv[3], O_RDONLY);
		if(fd_file == -1){
		  printf("Error opening file!\n");
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

	if (atoi(argv[2]) == 0){ //TRANSMITTER
		if(sendFile(fd_file, argv[3], fd) == -1) {
			printf("Error sending file.");
			return -1;
		}
	} else if (atoi(argv[2]) == 1){ //RECEIVER
		if(receiveFile(fd) == -1){
			printf("Error reading file.\n");
			return -1;
		}
	}

	if (llclose(fd, atoi(argv[2])) == -1) {
		perror("LLCLOSE");
		return -1;
	}

	displayStatistics();

	return 0;
}
