#include "app.h"
#include "ll.h"

App app;

int sendFile(int fd_file, char* file_name, int fd){

	//Gets file size
	struct stat buf;
	if (fstat(fd_file, &buf) == -1) {
		printf("ERROR: fstat");
		return -1;
	}

	app.sentFileSize = buf.st_size;
	app.sentFileName = file_name;
	app.serial_port_fd = fd;
	app.file_fd = fd_file;

	printf("\n***File info:***\n\n");
	printf("File Size: %d\n", app.sentFileSize);

	//Send Start Control Packet
	printf("\n***Sending start control packet:***\n\n");
	if (sendControlPacket(START_CONTROL_FIELD) == -1) {
		app.receivedFileSize = 0;
		printf("ERROR sending the first control packet!\n");
		return -1;
	}

	//Send Data Packets
	printf("\n***Sending data packets:***\n\n");
	if (sendDataPackets() == -1) {
		printf("ERROR sending the the data packets!\n");
		return -1;
	}

	//Send End Control Packet
	printf("\n***Sending end control packet:***\n\n");
	if (sendControlPacket(END_CONTROL_FIELD) == -1) {
		printf("ERROR sending the last control packet!\n");
		return -1;
	}

	return 0;

}

int sendControlPacket(unsigned char control_field){

	int index = 0;
	int file_size_length = sizeof(app.sentFileSize);
	unsigned char packet[CONTROL_PACKET_SIZE + file_size_length + strlen(app.sentFileName)];

	int bytesWritten = 0;

	packet[index++] = control_field;

	//INSERT FILE SIZE INFO
	packet[index++] = FILE_SIZE_FLAG;

	unsigned char byteArray[file_size_length];

	for (int i = 0; i < file_size_length; i++){
		byteArray[i] = (app.sentFileSize >> 8*(file_size_length - 1 - i)) & 0xFF;
	}

	packet[index++] = file_size_length;

	for (int i = 0; i < file_size_length; i++){
		packet[index++] = byteArray[i];
	}

	//INSERT FILE NAME INFO
	packet[index++] = FILE_NAME_FLAG;
	packet[index++] = strlen(app.sentFileName);

	for(size_t i = 0; i < strlen(app.sentFileName); i++) {
		packet[index++] = app.sentFileName[i];
	}

	printf("Sending Control Packet\n");
	bytesWritten = llwrite(app.serial_port_fd, packet, index);
	if (bytesWritten == -1) {
		printf("ERROR in llwrite!\n");
		return -1;
	}

	printf("Wrote %d control bytes\n", bytesWritten);
	printf("File name %s\n", app.sentFileName);
	printf("File name length %lu\n", strlen(app.sentFileName));

	return 0;
}

int sendDataPackets(){

	char buf[MAX_CHUNK_SIZE];
	int chunksSent = 0;
	int chunksToSend = app.sentFileSize / MAX_CHUNK_SIZE + (app.sentFileSize % MAX_CHUNK_SIZE != 0);
	int bytesRead = 0;

	int bytesWritten = 0;
	int totalBytesWritten = 0;

	printf("File size: %d\n", app.sentFileSize);
	printf("Chunks Sent: %d\n", chunksToSend);


	while (chunksSent < chunksToSend){
		bytesRead = read(app.file_fd, &buf, MAX_CHUNK_SIZE);
		unsigned char packet[DATA_PACKET_SIZE + bytesRead];

		packet[0] = DATA_FIELD;
		packet[1] = chunksSent % 255;
		packet[2] = bytesRead / 256;
		packet[3] = bytesRead % 256;
		memcpy(&packet[4], &buf, bytesRead);

		bytesWritten = llwrite(app.serial_port_fd, packet, bytesRead + DATA_PACKET_SIZE);
		if (bytesWritten == -1){
			printf("ERROR in llwrite!\n");
			return -1;
		}

		totalBytesWritten += bytesWritten;
		chunksSent++;
	}

	printf("Wrote %d bytes\n", totalBytesWritten);
	printf("Wrote %d data bytes\n", totalBytesWritten - DATA_PACKET_SIZE * chunksSent);
	return 0;
}

int receiveFile(int fd){

	unsigned char max_buf[MAX_CHUNK_SIZE + DATA_PACKET_SIZE];
	int bytesRead = 0;
	int received = 0;
	int llAux = 0;

	app.serial_port_fd = fd;

	printf("\n***Reading Control Packet***\n\n");

	app.created_file_fd = readControlPacket();

	printf("***Reading Data Packets***\n\n");

	while (!received) {
		if((llAux = llread(fd, max_buf)) != 0) {
			bytesRead += llAux;
			if(max_buf[0] == DATA_FIELD)
				readDataPackets(max_buf);
			else if (max_buf[0] == END_CONTROL_FIELD)
				received = 1;
		}
	}

	printf("\n");

	close(app.created_file_fd);

	return 0;
}

int readControlPacket(){

	int index = 1;
	int file_size = 0;
	char* file_name;

	unsigned char packet[MAX_CHUNK_SIZE];
	llread(app.serial_port_fd, packet);

	printf("\n***File info:***\n\n");

	//FILE SIZE
	if(packet[index] == FILE_SIZE_FLAG){
		index++;
		int size_length = packet[index];
		index++;

		for (int i = 0; i < size_length; i++){
			file_size += packet[index] << 8 * (size_length - 1 - i);
			index++;
		}

		if (app.receivedFileSize != 0){
			if(app.receivedFileSize == file_size){
				printf("Start Control Packet and End Control Packet data does not match\n");
				return -1;
			}
		}

		printf("File size: %d bytes\n", file_size);
	}
	if (file_size <= 0) {
		perror("File size error\n");
		return -1;
	}

	app.receivedFileSize = file_size;

	//FILE NAME
	if (packet[index] == FILE_NAME_FLAG) {
		index++;
		int name_length = packet[index];
		index++;

		file_name = (char*) malloc(name_length + 1);
		for (int i = 0; i < name_length; i++) {
			file_name[i] = packet[index];
			index++;
		}

		file_name[name_length] = '\0';

		if (app.receivedFilename != NULL){
			if(app.receivedFilename == file_name){
				printf("Start Control Packet and End Control Packet data does not match\n");
				return -1;
			}
		}

		printf("File Name: %s\n\n", file_name);
	}

	app.receivedFilename = file_name;

	app.serial_port_fd = open(app.receivedFilename, O_WRONLY | O_CREAT | O_APPEND);

	return app.serial_port_fd;
}

int readDataPackets(unsigned char* packet){

	printf("Writing data packets to file\n");

	int dataSize = 256 * packet[2] + packet[3];
	printf("Packet size: %d bytes\n\n", dataSize);

	write(app.created_file_fd, &packet[4], dataSize);

	return 0;
}
