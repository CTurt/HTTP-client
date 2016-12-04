#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define HOST "cturt.github.io"
#define PAGE "/"
#define PORT 80
#define USERAGENT "HAX"

char *constructQuery(const char *host, const char *page, size_t *restrict size) {
	char *query;
	
	const char *http = "GET /%s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: %s\r\n"
		"Connection: close\r\n"
		"\r\n";
	
	page += page[0] == '/';
	
	FILE *null = fopen("/dev/null", "w");
	*size = fprintf(null, http, page, host, USERAGENT);
	fclose(null);
	
	query = malloc(*size + 1);
	sprintf(query, http, page, host, USERAGENT);
	
	return query;
}

int main(int argc, char **argv) {
	int sock;
	struct sockaddr_in address;
	struct addrinfo hints, *res;
	
	char *query;
	size_t querySize;
	
	char *buffer;
	ssize_t r;
	
	const char *host = HOST;
	const char *page = PAGE;
	
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		exit(1);
	}
	
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	if(getaddrinfo(host, NULL, &hints, &res) != 0) {
		perror("getaddrinfo");
		exit(1);
	}
	
	if(res->ai_family == AF_INET) {
		address.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
	}
	else {
		perror("Address family not AF_INET!");
		exit(1);
	}
	
	freeaddrinfo(res);
	
	if(connect(sock, (struct sockaddr *)&address, sizeof(struct sockaddr)) < 0) {
		perror("connect");
		exit(1);
	}
	
	query = constructQuery(host, page, &querySize);
	//printf("Query:\n%s", query);
	
	size_t sent = 0;
	while(sent < querySize) {
		if((r = send(sock, query + sent, querySize - sent, 0)) == -1) {
			perror("send");
			exit(1);
		}
		
		sent += r;
	}
	
	free(query);
	
	buffer = malloc(BUFSIZ);
	memset(buffer, 0, BUFSIZ);
	
	while((r = recv(sock, buffer, BUFSIZ, 0)) > 0) {
		//"\r\n\r\n"
		fwrite(buffer, (int)r, 1, stdout);
	}
	
	free(buffer);
	
	close(sock);
	
	return 0;
}
