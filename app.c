#include "app.h"

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

int sendControlPacket(int control_field, int file_size, char* file_name, int fd){

	int index = 0;
	char packet[CONTROL_PACKET_SIZE + sizeof(file_size) + strlen(file_name)];

	int bytesWritten = 0;

	packet[index++] = control_field;

	//INSERT FILE SIZE INFO
	packet[index++] = FILE_SIZE_FLAG; //Type1

	unsigned char byteArray[sizeof(file_size)];

	for (int i = 0; sizeof(byteArray); i++){
		byteArray[i] = (file_size >> 8*(sizeof(byteArray) - 1)) & 0x0FF;
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
		bytesRead = read(fd_file, &buf, MAX_CHUNK_SIZE);
		char packet[DATA_PACKET_SIZE + bytesRead];
		
		packet[0] = DATA_FIELD;
		packet[1] = chunksSent % 255;
		packet[2] = bytesRead / 256;
		packet[3] = DATA_PACKET_SIZE % 256;
		memcpy(packet[4], &buf, bytesRead);

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

	char max_buf[MAX_CHUNK_SIZE + DATA_PACKET_SIZE];
	int bytesRead = 0;
	int received = 0;

	int received_fd_file = 0;

	while (!received){

		bytesRead += llread(fd, max_buf);

		if (max_buf[0] == DATA_FIELD){
			readDataPackets(max_buf, received_fd_file);
		} else if (max_buf[0] == START_CONTROL_FIELD){
			readControlPacket(max_buf);
		} else if (max_buf[0] == END_CONTROL_FIELD){
			received = 1;
		}
	}

	close(received_fd_file);

	return 0;

}

int readControlPacket(char* packet){

	int index = 1;

	int file_size;
	char file_name;

	if(packet[index] == FILE_SIZE_FLAG){ //FILE SIZE
		index++;
		int size_length = packet[index];
		index++;

		for (int i = 0; i < size_length; i++){
			file_size += packet[index] << 8 * (size_length - 1);
			index++;
		}
		index++;
	} 
	
	if (packet[index] == FILE_NAME_FLAG) { //FILE NAME
		index++;
		int name_length = packet[index];
		index++;

		memcpy(file_name, &packet[index], name_length);
	}

	if(file_size == NULL || file_name == NULL){
		printf("ERROR reading file's size or name\n");
		return -1;
	}

	return 0;
	
}

int readDataPackets(char* packet, int fd_file){

	int dataSize = 256 * packet[2] + packet[3];

	write(fd_file, packet, dataSize);

	return 0;

}