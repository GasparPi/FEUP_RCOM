#include "url_handler.h"

void create_url_struct(url_t* url) {
    memset(url->user, 0, MAX_STRING_SIZE);
	memset(url->password, 0, MAX_STRING_SIZE);
	memset(url->host_name, 0, MAX_STRING_SIZE);
    memset(url->ip_address, 0, MAX_STRING_SIZE);
	memset(url->url_path, 0, MAX_STRING_SIZE);
	memset(url->filename, 0, MAX_STRING_SIZE);
	url->port = 21;
}

int get_url(url_t* url, const char* str) {

    //TODO

}

int get_ip_address(url_t* url) {
    struct hostent* h;

    if ((h=gethostbyname(url->host_name)) == NULL) {  
        herror("gethostbyname");
        return 1;
    }

    char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));
	strcpy(url->ip_address, ip);

	return 0;
}


