#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FIELD 1
#define START_CONTROL_FIELD 2
#define END_CONTROL_FIELD 3

#define DATA_PACKET_SIZE 4
#define CONTROL_PACKET_SIZE 5
#define FILE_SIZE_FLAG 0
#define FILE_NAME_FLAG 1

#define MAX_CHUNK_SIZE 1024

int sendFile(int fd_file, char* file_name, int fd);
int receiveFile(int fd);

int sendControlPacket(int control_field, int file_size, char* file_name, int fd);
int sendDataPackets(int file_size, int fd_file, int fd);

int readControlPackets(char* packet);
int readDataPackets(char* packet, int fd_file);
