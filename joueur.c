/*
 * ==================================================================
 *
 * Filename : joueur.c
 *
 * Description : file used to play the client role.
 *
 * Author : MANIET Antoine "amaniet152", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */

#include "global.h"
#include "joueur.h"

int main(int argc, char** argv) {
	SOCKET sock;
	message buffer;
	char ligne[1024];
	char name[NAME_LENGTH];
	const char *hostname;
	struct hostent *hostinfo = NULL;
	int n = 0;
	int port;
	SOCKADDR_IN sin = { 0 };
	if(argc != 3) {
		fprintf(stderr, "joueur [port] [ipHost]\n");
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

	keyboardReader(&name);	
	printf("%s", buffer.content);
	strlcpy(buffer.content, name, strlen(name)+1);
	buffer.status = 200;
	/* ENVOI */
	if(send(sock, &buffer, sizeof(buffer), 0) < 0) {
		perror("send()");
		exit(errno);
	}
	if (buffer.status == 500) exit(0);
	printf("Vous êtes actuellement en attente d'une réponse du serveur… …\n");
	/* RECEPTION MESSAGE */
	if ((n = recv(sock, &buffer, sizeof(buffer) -1, 0)) < 0) {
		exit(errno);
		perror("recv()");
	}
	printf("%s", buffer.content);
	switch(buffer.status){
	case 201 :
		/* ENVOI MESSAGE 201 */
		buffer.status = 200;
		memset(buffer.content, 0, sizeof buffer.content);

		sprintf(buffer.content, "Selection des cartes = BLABLA\n");
		if(send(sock, &buffer, sizeof(buffer), 0) < 0) {
			perror("send()");
			exit(errno);
		}
		printf("message envoyé");
		break;
	default :
		/* ENVOI MESSAGE ERREUR */
		buffer.status = 400;
		memset(buffer.content, 0, sizeof buffer.content);
		sprintf(buffer.content, "Incomprehension du status du message\n");
		if(send(sock, &buffer, sizeof(buffer), 0) < 0) {
			perror("send()");
			exit(errno);
		}
		printf("erreur envoyée");
	}
	closesocket(sock);
}

int keyboardReader(char* name[NAME_LENGTH]){
	if (fgets(name, NAME_LENGTH, stdin) == NULL){
		fprintf(stderr, "fgets problem");
		exit(EXIT_FAILURE);
	}
	fflush(stdin);
}
