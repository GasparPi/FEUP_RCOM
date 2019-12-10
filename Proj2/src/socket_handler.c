#include "socket_handler.h"

int open_socket(const char* ip_address, const int port) {
    int socket_fd;
	struct sockaddr_in server_addr;

	// server address handling
	bzero((char*) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_address); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port); /*server TCP port must be network byte ordered */

	// open a TCP socket
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket()\n");
		return -1;
	}

	// connect to the server
	if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("connect()\n");
		return -1;
	}

	return socket_fd;
}

int write_to_socket(const int socket_fd, const char* str, const size_t str_size) {
    int bytes;

    // Write a string to the server
	if ((bytes = write(socket_fd, str, str_size)) <= 0) {
		perror("Write to socket\n");
		return 1;
	}

	printf("Bytes written to server: %d\nInfo: %s\n", bytes, str);

	return 0;
}

int read_from_socket(const int socket_fd, char* str, size_t str_size) {
    FILE* fp = fdopen(socket_fd, "r");

	do {
		memset(str, 0, str_size);
		str = fgets(str, str_size, fp);
		printf("%s", str);

	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}



