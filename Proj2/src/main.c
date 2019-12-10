#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "url_handler.h"
#include "ftp_connecter.h"

int main(int argc, char** argv) {

    if(argc != 2) {
        perror("Usage: wrong number of arguments\n");
        exit(1);
    }

    url_t url;
    create_url_struct(&url);

    if (get_url_info(&url, argv[1])) {
        perror("Error in URL syntax!\n");
        exit(1);
    }

    if (get_ip_address(&url)) {
        perror("Getting IP address by host name\n");
        exit(1);
    }

    print_url(&url);


    ftp_t ftp;
    if (ftp_init_connection(&ftp, url.ip_address, url.port)) {
        perror("ftp_init_connection()\n");
        exit(1);
    }
    printf("Started ftp connection successfully\n");
        
    if (ftp_login(&ftp, url.user, url.password)) {
        perror("ftp_login()");
        exit(1);
    }
    printf("Loged in successfully\n");
/*
    if (ftp_cwd(&ftp, url.url_path)) {
		perror("ftp_cwd()");
		exit(1);
	}
    printf("Changed directory successfully\n");*/

    if (ftp_passive_mode(&ftp)) {
        perror("ftp_passive_mode()\n");
        exit(1);
    }
    printf("Entered passive mode successfully\n");

    char filepath[MAX_BUFFER_SIZE];
    sprintf(filepath, "%s%s", url.url_path, url.filename);
    printf("Filepath: %s\n", filepath);

    if (ftp_retr_file(&ftp, filepath)) {
        perror("ftp_retr_file()");
        exit(1);
    }
    printf("Command retr 'file' executed successfully\n");

    if (ftp_download_file(&ftp, url.filename)) {
        perror("ftp_download_file()\n");
        exit(1);
    }
    printf("Downloaded file successfully\n");

    //disconnect

    return 0;
}

