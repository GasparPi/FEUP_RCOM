#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>	
#include <signal.h>

#define START_CONTROL_FIELD 1
#define START_CONTROL_FIELD 2
#define END_CONTROL_FIELD 3

int sendFile(int fd_file, char* file_name);
int receiveFile();

int sendControlPacket(int control_field, int file_size, char* file_name);
