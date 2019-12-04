#include "tcp_connecter.h"

int ftp_init_connection(ftp_t* ftp, char* ip_address, int port) {
    int socket_fd;
	char buffer[MAX_BUFFER_SIZE];

	if ((socket_fd = open_socket(ip_address, port)) < 0) {
		perror('open_socket()');
		return 1;
	}

	if (read_from_socket(ftp->control_socket_fd, buffer, sizeof(buffer))) {
		perror('ftp_read()');
		return 1;
	}

    ftp->control_socket_fd = socket_fd;
	ftp->data_socket_fd = 0;

	return 0;
}

int ftp_user_command(ftp_t* ftp, char* user, char* password) {

}


