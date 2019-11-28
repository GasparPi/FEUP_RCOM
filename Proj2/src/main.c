#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "url_handler.h"

int main(int argc, char** argv) {

    if(argc != 2) {
        perror("usage: getip address\n");
        exit(1);
    }

    url_t url;
    create_url_struct(&url);

    if (get_url(&url, argv[1])) {
        perror("Reading URL");
        exit(1);
    }

    if (get_ip_address(&url)) {
        perror("Getting IP address by host name");
        exit(1);
    }
        






    return 0;
}