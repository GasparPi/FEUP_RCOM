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
#define FILE_SIZE_FLAG 0
#define FILE_NAME_FLAG 1

#define MAX_CHUNK_SIZE 1024

int sendFile(int fd_file, char* file_name, int fd);
int receiveFile(int fd);

int sendControlPacket(unsigned char control_field, int file_size, char* file_name, int fd);
int sendDataPackets(int file_size, int fd_file, int fd);

int readControlPacket(unsigned char* packet, int* fd);
int readDataPackets(unsigned char* packet, int fd_file);
