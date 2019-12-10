#pragma once

#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_STRING_SIZE 256
#define h_addr h_addr_list[0]	//The first address in h_addr_list. 

typedef struct url_t {
	char user[MAX_STRING_SIZE]; 
	char password[MAX_STRING_SIZE]; 
	char host_name[MAX_STRING_SIZE]; 
	char ip_address[MAX_STRING_SIZE]; 
	char url_path[MAX_STRING_SIZE]; 
	char filename[MAX_STRING_SIZE]; 
	int port; 
} url_t;

void create_url_struct(url_t* url);
int get_url_info(url_t* url, char* str);
int get_ip_address(url_t* url);

// Auxiliary functions
int check_ftp(const char* str);
int check_username(const char* str);
int get_username(const char* str, char* username);
int get_password(const char* str, char* password);
int get_host_name(const char* url_rest, char* host_name);
int get_url_path(const char* str, char* url_path, char* filename);
char* get_str_before_char(const char* str, const char chr);
void print_url(url_t* url);


