#include "ftp_connecter.h"

int ftp_init_connection(ftp_t* ftp, const char* ip_address, const int port) {

    int socket_fd;
	char buffer[MAX_BUFFER_SIZE];

	if ((socket_fd = open_socket(ip_address, port)) < 0) {
		perror("open_socket()\n");
		return 1;
	}

	ftp->control_socket_fd = socket_fd;
	ftp->data_socket_fd = 0;

	if (read_from_socket(ftp->control_socket_fd, buffer, sizeof(buffer))) {
		perror("ftp_read()");
		return 1;
	}

	return 0;
}

int ftp_command_with_response(const int socket_fd, const char* command, char* response) {

	// Buffer used to temporarily store comunication messages
	char buffer[MAX_BUFFER_SIZE]; 
	
	// User command
	sprintf(buffer, "%s\r\n", command);
	if (write_to_socket(socket_fd, buffer, strlen(buffer))) {
		perror("write_to_socket()\n");
		return 1;
	}

	// Reading response of user command
	if (read_from_socket(socket_fd, buffer, sizeof(buffer))) {
		perror("read_from_socket()\n");
		return 1;
	}

	memcpy(response, buffer, sizeof(buffer));

	return 0;
}

int ftp_command(const int socket_fd, const char* command) {
	
	char response[MAX_BUFFER_SIZE]; 
	ftp_command_with_response(socket_fd, command, response);

	return 0;
}

int ftp_login(const ftp_t* ftp, const char* user, const char* password) {

    char command[MAX_COMMAND_SIZE];

	// User command
	sprintf(command, "USER %s", user);
	if (ftp_command(ftp->control_socket_fd, command)) {
		perror("ftp_command()\n");
		return 1;
	}

	// Cleaning command buffer
	memset(command, 0, sizeof(command));

	// Password command
	sprintf(command, "PASS %s", password);
	if (ftp_command(ftp->control_socket_fd, command)) {
		perror("ftp_command()\n");
		return 1;
	}

	return 0;
}

int ftp_passive_mode(ftp_t* ftp) {

	char command[MAX_BUFFER_SIZE];
	char response[MAX_BUFFER_SIZE];
	
	sprintf(command, "PASV");
	if (ftp_command_with_response(ftp->control_socket_fd, command, response)) {
		perror("ftp_command()\n");
		return 1;
	}

	char ip_address[MAX_BUFFER_SIZE];
	int port_num;

	if (ftp_process_pasv_response(response, ip_address, &port_num)) {
		perror("ftp_process_pasv_response()\n");
		return 1;
	}

	printf("IP: %s\n", ip_address);
	printf("PORT: %d\n", port_num);

	if ((ftp->data_socket_fd = open_socket(ip_address, port_num)) < 0) {
		perror("open_socket()\n");
		return 1;
	}

	return 0;
}

int ftp_process_pasv_response(const char* response, char* ip_address, int* port_num) {

	int ip1, ip2, ip3, ip4;
	int port1, port2;

	if (sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2) < 0) {
		perror("sscanf()\n");
		return 1;
	}

	// Creating ip address
	sprintf(ip_address, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

	// Calculating new port number
	*port_num = port1 * 256 + port2;

	return 0;
}

int ftp_retr_file(const ftp_t* ftp, const char* filename) {
	char command[MAX_BUFFER_SIZE];

	sprintf(command, "RETR %s", filename);
	if (ftp_command(ftp->control_socket_fd, command)) {
		perror("ftp_command()\n");
		return 1;
	}

	return 0;
}

int ftp_download_file(ftp_t* ftp, const char* filename) {

	char buffer[MAX_BUFFER_SIZE];
	int file_fd;
	int bytes_read;

	if((file_fd = open(filename, O_WRONLY | O_CREAT, 0666)) < 0) {
		perror("open()\n");
		return 1;
	}

	while ((bytes_read = read(ftp->data_socket_fd, buffer, sizeof(buffer)))) {

		if (bytes_read < 0) {
			perror("read()\n");
			return 1;
		}

		if (write(file_fd, buffer, bytes_read) < 0) {
			perror("write()\n");
			return 1;
		}

	}

	close(file_fd);
	close(ftp->data_socket_fd);

	return 0;
}


