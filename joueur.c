/*
 * ==================================================================
 *
 * Filename : joueur.c
 *
 * Description : file used to play the client role.
 * 
 * Author : MANIET Antoine "nomLogin", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */

#include "global.h"
#include "joueur.h"

int main(int argc, char** argv) {
	SOCKET sock;
	message buffer;
	char ligne[1024];
	const char *hostname;
	struct hostent *hostinfo = NULL;
	int n = 0;
	int port;
	SOCKADDR_IN sin = { 0 };
	if(argc != 3) {
		fprintf(stderr, "joueur [port] [ip]\n");
		return EXIT_ERROR;
	}
	port = atoi(*++argv);
	hostname = *++argv;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) {
		perror("socket()");
		exit(errno);
	}
	hostinfo = gethostbyname(hostname);
	if(hostinfo == NULL) {
		fprintf(stderr, "Unknown host %s.\n", hostname);
		exit(EXIT_FAILURE);
	} 

	sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
	sin.sin_port = htons(port);
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
