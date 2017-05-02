#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#define PORT 8888

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

typedef struct message {
	int status;
	char content[1024];
} message;

int main(int argc, char** argv) {
	SOCKET sock;
	message buffer;
	char ligne[1024];
	const char *hostname = "127.0.0.1";
	struct hostent *hostinfo = NULL;
	int n = 0;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) {
		perror("socket()");
		exit(errno);
	}
	SOCKADDR_IN sin = { 0 };
	hostinfo = gethostbyname(hostname);
	if(hostinfo == NULL) {
		fprintf(stderr, "Unknown host %s.\n", hostname);
		exit(EXIT_FAILURE);
	} 

	sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
	sin.sin_port = htons(PORT);
	sin.sin_family = AF_INET;
	if(connect(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		perror("connect()");
		exit(errno);
	}
	if ((n = recv(sock, &buffer, sizeof(buffer) -1, 0)) < 0) {
		exit(errno);
		perror("recv()");
	}
	printf("%d\n", n);
	printf("%s", buffer.content);
	sprintf(buffer.content, "Jean\n");
	buffer.status = 200;
	/* ENVOI */
	if(send(sock, &buffer, sizeof(buffer), 0) < 0) {
		perror("send()");
		exit(errno);
	}
	closesocket(sock);
}
