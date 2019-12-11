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

int get_url_info(url_t* url, const char* str) {

    // str = ftp://[<user>:<password>@]<host>/<url-path>
    if (!check_ftp(str))
        return 1;

    char* temp_url = (char*) malloc(strlen(str));
    char* url_path = (char*) malloc(strlen(str));
	memcpy(temp_url, str, strlen(str));
    
    // removing ftp:// from string
	strcpy(temp_url, temp_url + 6); // temp_url = [<user>:<password>@]<host>/<url-path>

    char* url_rest = strchr(temp_url, '@');

    if (url_rest == NULL) {
        printf("User not defined\n");

        strcpy(url_path, temp_url);  // url_path = <host>/<url-path>

        strcpy(url->user, "anonymous");
        strcpy(url->password, "any");
    }
    else {
        printf("User defined\n");

        strcpy(url_path, url_rest + 1); // url_path = <host>/<url-path>

        get_username(temp_url, url->user);
        printf("Username obtained: %s\n", url->user);

        strcpy(temp_url, temp_url + strlen(url->user) + 1); // temp_url = <password>@<host>/<url-path>

        get_password(temp_url, url->password);
        printf("Password obtained: %s\n", url->password);
    }

    get_host_name(url_path, url->host_name);
    printf("Host name obtained: %s\n", url->host_name);

    strcpy(url_path, url_path + strlen(url->host_name) + 1); // url_rest = /<url-path>

    get_url_path(url_path, url->url_path, url->filename);
    printf("URL path obtained: %s\n", url->url_path);

    return 0;
}

int get_ip_address(url_t* url) {
    struct hostent* h;

    if ((h = gethostbyname(url->host_name)) == NULL) {  
        perror("gethostbyname()\n");
        return 1;
    }

    char* ip = inet_ntoa(*( (struct in_addr *) h->h_addr) );
	strcpy(url->ip_address, ip);

	return 0;
}

int check_ftp(const char* str) {

    char* ftp_str = "ftp://";
    char substring[6];

    memcpy(substring, str, 6);

    return !strcmp(ftp_str, substring);
}

int get_username(const char* str, char* username) {
    strcpy(username, get_str_before_char(str, ':'));

    return 0;
}

int get_password(const char* str, char* password) {
    strcpy(password, get_str_before_char(str, '@'));

    return 0;
}

int get_host_name(const char* str, char* host_name) {
    strcpy(host_name, get_str_before_char(str, '/'));

    return 0;
}

int get_url_path(const char* str, char* url_path, char* filename) {

    char* path = (char*) malloc(strlen(str));

    char* working_str = (char*) malloc(strlen(str));
    memcpy(working_str, str, strlen(str));
    char* temp_str = (char*) malloc(strlen(str));

	while (strchr(working_str, '/')) {
		
        temp_str = get_str_before_char(working_str, '/');
        strcpy(working_str, working_str + strlen(temp_str) + 1);

		strcpy(path, temp_str);
		strcat(path, "/");
	}

	strcpy(url_path, path);
	strcpy(filename, working_str);

	free(path);
    free(temp_str);

    return 0;
}

char* get_str_before_char(const char* str, const char chr) {

	char* temp = (char*) malloc(strlen(str));
	int index = strlen(str) - strlen(strcpy(temp, strchr(str, chr)));

	temp[index] = '\0'; 
	strncpy(temp, str, index);

	return temp;
}

void print_url(url_t* url) {
    printf("USER: %s\n", url->user);
    printf("PASSWORD: %s\n", url->password);
    printf("HOST: %s\n", url->host_name);
    printf("IP: %s\n", url->ip_address);
    printf("PATH: %s\n", url->url_path);
    printf("FILE: %s\n\n\n", url->filename);
}


