#pragma once

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>

#include "socket_handler.h"

typedef struct ftp_t {
    int control_socket_fd;
    int data_socket_fd;
} ftp_t;

#define MAX_BUFFER_SIZE 1024
#define MAX_COMMAND_SIZE 50

int ftp_init_connection(ftp_t* ftp, const char* ip_addres, const int port);
int ftp_command(const int socket_fd, const char* command);
int ftp_command_with_response(const int socket_fd, const char* command, char* response);
int ftp_login(const ftp_t* ftp, const char* user, const char* password);
int ftp_passive_mode(ftp_t* ftp);
int ftp_retr_file(const ftp_t* ftp, const char* filename);
int ftp_download_file(ftp_t* ftp, const char* filename);

// Auxiliay functions
int ftp_process_pasv_response(const char* response, char* ip_address, int* port_num);

