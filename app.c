#include "app.h"
#include "ll.h"

int sendFile(int fd_file, char* file_name, int fd){

	int file_size;

	//Gets file size
	struct stat buf;
	if (fstat(fd_file, &buf) == -1) {
		printf("ERROR: fstat");
		return -1;
	}

	file_size = buf.st_size;

	printf("File Size: %d\n", file_size);

	//Send Start Control Packet
	if (sendControlPacket(START_CONTROL_FIELD, file_size, file_name, fd) == -1) {
		printf("ERROR sending the first control packet!\n");
		return -1;
	}

	//Send Data Packets
	if (sendDataPackets(file_size, fd_file, fd) == -1) {
		printf("ERROR sending the the data packets!\n");
		return -1;
	}

	//Send End Control Packet
	if (sendControlPacket(END_CONTROL_FIELD, file_size, file_name, fd) == -1) {
		printf("ERROR sending the last control packet!\n");
		return -1;
	}

	return 0;

}

int sendControlPacket(unsigned char control_field, int file_size, char* file_name, int fd){

	int index = 0;
	char packet[CONTROL_PACKET_SIZE + sizeof(file_size) + strlen(file_name)];

	int bytesWritten = 0;

	packet[index++] = control_field;

	//INSERT FILE SIZE INFO
	packet[index++] = FILE_SIZE_FLAG; //Type1

	unsigned char byteArray[sizeof(file_size)];

	for (int i = 0; i < sizeof(byteArray); i++){
		byteArray[i] = (file_size >> 8*i) & 0x0FF;
	}

	packet[index++] = strlen((char *) byteArray);

	for (int i = 0; i < strlen((char *) byteArray) ; i++){
		packet[index++] = byteArray[i];
	}

	//INSERT FILE NAME INFO
	packet[index++] = FILE_NAME_FLAG;
	packet[index++] = sizeof(file_name);

	for(int i = 0; i < strlen(file_name); i++) {
		packet[index++] = file_name[i];
	}

	printf("Sending Control Packet\n");
	bytesWritten = llwrite(fd, packet, index);
	if (bytesWritten == -1){
		printf("ERROR in llwrite!\n");
		return -1;
	}

	printf("Wrote %d control bytes\n", bytesWritten);
	return 0;
}

int sendDataPackets(int file_size, int fd_file, int fd){

	char buf[MAX_CHUNK_SIZE];
	int chunksSent = 0;
	int chunksToSend = ceil(file_size/MAX_CHUNK_SIZE);
	int bytesRead = 0;

	int bytesWritten = 0;
	int totalBytesWritten = 0;

	while (chunksSent < chunksToSend){
		printf("before read in send data packets\n");
		bytesRead = read(fd_file, &buf, MAX_CHUNK_SIZE);
		printf("after read in send data packets\n");;
		char packet[DATA_PACKET_SIZE + bytesRead];

		packet[0] = DATA_FIELD;
		packet[1] = chunksSent % 255;
		packet[2] = bytesRead / 256;
		packet[3] = DATA_PACKET_SIZE % 256;
		memcpy(&packet[4], &buf, bytesRead);

		printf("Sending Data Packet\n");
		bytesWritten = llwrite(fd, packet, strlen(packet));
		if (bytesWritten == -1){
			printf("ERROR in llwrite!\n");
			return -1;
		}

		totalBytesWritten += bytesWritten;
		chunksSent++;
	}

	printf("Wrote %d data bytes\n", totalBytesWritten);
	return 0;
}

int receiveFile(int fd){

	unsigned char max_buf[MAX_CHUNK_SIZE + DATA_PACKET_SIZE];
	int bytesRead = 0;
	int received = 0;

	int new_file_fd;
	while (!received) {

		bytesRead += llread(fd, max_buf);

		if (max_buf[0] == DATA_FIELD){
			readDataPackets(max_buf, new_file_fd);
		} else if (max_buf[0] == START_CONTROL_FIELD){
			readControlPacket(max_buf, &new_file_fd);
		} else if (max_buf[0] == END_CONTROL_FIELD){
			received = 1;
		}
	}

	close(new_file_fd);

	return 0;

}

int readControlPacket(unsigned char* packet, int* fd){

	int index = 1;
	int file_size;
	char* file_name;

	//FILE SIZE
	if(packet[index] == FILE_SIZE_FLAG){
		index++;
		int size_length = packet[index];
		index++;

		for (int i = 0; i < size_length; i++){
			file_size += packet[index] << 8 * (size_length - 1);
			index++;
		}
		index++;
	}
	if (file_size <= 0) {
		perror("File size error\n");
		return -1;
	}

	if (packet[index] == FILE_NAME_FLAG) { //FILE NAME
		index++;
		int name_length = packet[index];
		printf("Name length: %d\n", name_length);
		index++;

		file_name = (char*) malloc(name_length);
		memcpy(&file_name, &packet[index], name_length);
		file_name[name_length] = '\0';

		printf("Name: %s\n", file_name);
	}

	*fd = open(file_name, O_WRONLY | O_CREAT);

	return 0;

}

// TODO review
int readDataPackets(unsigned char* packet, int fd_file){

	int dataSize = 256 * packet[2] + packet[3];

	write(fd_file, packet, dataSize);

	return 0;

}
