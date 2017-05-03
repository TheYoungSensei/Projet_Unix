/*
 * ==================================================================
 *
 * Filename : serveur.c
 *
 * Description : file used to play the server role.
 * 
 * Author : MANIET Antoine "nomLogin", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */

#include "global.h"
#include "serveur.h"

int main(int argc, char** argv) {
	SOCKET sock;
	SOCKET csock[MAX_PLAYER];
	char * pseudos[4];
	message buffer;
	int port, n = 0, i = 0;
	const char *hostname = "127.0.0.1";
	SOCKADDR_IN sin;
	SOCKADDR_IN csin;
	if(argc != 3) {
		fprintf(stderr, "serveur [numPort] [stderr]\n");
		return EXIT_ERROR;
	}
	port = atoi(*++argv);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) {
		perror("socket()");
		exit(errno);
	}
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
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
		if((pseudos[i] = (char *) malloc(sizeof(char) * strlen(buffer))) == -1) {
			perror("malloc()");
			exit(errno);
		}
		pseudos[i] = buffer;
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
