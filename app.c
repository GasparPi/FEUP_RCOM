#include "app.h"

int sendFile(int fd_file, char* file_name){

	int file_size;

	//Gets file size
	struct stat buf;
	if (fstat(fd_file, &buf) == -1) {
		printf("ERROR: fstat");
		return -1;
	}

	file_size = buf.st_size;

	printf("File Size: %d\n", file_size);

	//Send 1st Control Packet
	sendControlPacket(START_CONTROL_FIELD, file_size, file_name);

	//Send Data Packet

	//Send 2nd Control Packet
	sendControlPacket(END_CONTROL_FIELD, file_size, file_name);

	return 0;	
		
}

int receiveFile(){
	printf("ola");
}

int sendControlPacket(int control_field, int file_size, char* file_name){
	
}

