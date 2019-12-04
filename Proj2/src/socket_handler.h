#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>

int open_socket(char* ip_address, int port);
int write_to_socket(int socket_fd, char* str, size_t str_size);
int read_from_socket(int socket_fd, char* str, size_t str_size);

