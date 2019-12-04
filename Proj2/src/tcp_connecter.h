#pragma once

#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "socket_handler.h"

typedef struct ftp_t {
    int control_socket_fd;
    int data_socket_fd;
} ftp_t;

#define MAX_BUFFER_SIZE 1024

int ftp_init_connection(ftp_t* ftp, char* ip_addres, int port);
int ftp_user_command(ftp_t* ftp, char* user, char* password);


