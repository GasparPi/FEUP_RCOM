#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "ll.h"

#define DATA_FIELD 0x01
#define START_CONTROL_FIELD 0x02
#define END_CONTROL_FIELD 0x03

#define DATA_PACKET_SIZE 4
#define CONTROL_PACKET_SIZE 5
#define FILE_SIZE_FLAG 0x00
#define FILE_NAME_FLAG 0x01

#define MAX_CHUNK_SIZE 1024

typedef struct {
    char* sentFileName;
	char* receivedFilename;
	int sentFileSize;
	int receivedFileSize;
    //Sent image's file descriptor
	int file_fd;
    //Serial Port's file descriptor
	int serial_port_fd;
    //Received image's file descriptor
	int created_file_fd;
} App;

int sendFile(int fd_file, char* file_name, int fd);
int receiveFile(int fd);

int sendControlPacket(unsigned char control_field);
int sendDataPackets();

int readControlPacket();
int readDataPackets(unsigned char* packet);
