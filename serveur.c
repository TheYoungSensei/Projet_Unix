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
#define MAX_PLAYER 5

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
	SOCKET csock[MAX_PLAYER];
	message buffer;
	const char *hostname = "127.0.0.1";
	int n = 0;
	int i = 0;
	SOCKADDR_IN sin;
	SOCKADDR_IN csin;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) {
		perror("socket()");
		exit(errno);
	}
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	if(bind(sock, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR) {
		perror("bind()");
		exit(errno);
	}
	if(listen(sock, MAX_PLAYER) == SOCKET_ERROR) {
		perror("listen()");
		exit(errno);
	}
	int sinsize = sizeof csin;
	while((csock[i] = accept(sock, (SOCKADDR *) &csin, (socklen_t *) &sinsize)) && i < MAX_PLAYER) {
		buffer.status = 200;
		sprintf(buffer.content, "Bienvenue sur notre Papayoo's party\n Il y a actuellement %d joueur(s) connectés\n", i + 1); 
		if(send(csock[i], &buffer, sizeof(buffer) -1, 0) < 0) {
			perror("send()");
			exit(errno);
		}
		/* RECEPTION */
		if((n = recv(csock[i], &buffer, sizeof(buffer) - 1, 0)) < 0) {
			perror("recv()");
			exit(errno);
		}
		i++;
		printf("Tentative de connexion de : %s\n", buffer.content);
	}
	if(csock[i] == INVALID_SOCKET) {
		perror("accept()");
		exit(errno);
	}
	closesocket(sock);
	for(i =0; i < MAX_PLAYER; i++) {
		closesocket(csock[i]);
	}
}
