/*
 * ==================================================================
 *
 * Filename : joueur.c
 *
 * Description : file used to play the client role.
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */

#include "global.h"
#include "joueur.h"
#include "socket.h"
#include "sharedMemory.h"

int main(int argc, char** argv) {
	SOCKET sock;
	message buffer;
	char ligne[1024];
	char * name;
	const char *hostname;
	int n = 0, port;
	SOCKADDR_IN sin = { 0 };
	if(argc != 3) {
		fprintf(stderr, "joueur <port> <ipHost>\n");
		return ERROR;
	}
	port = atoi(*++argv);
	hostname = *++argv;
	sock = joueurInit(hostname, &sin, port);
	/* Trying to connect to the server */
	if(connect(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == ERROR) {
		perror("connect()");
		exit(errno);
	}
	/* Showing Welcome message */
	readJ(sock, &buffer);
	printf("%s", buffer.content);
	/* readJing the userName */
	printf("Veuillez entrer votre pseudo : \n");
	fflush(stdin);
	keyboardReader(&name);
	strcpy(buffer.content, name);
	buffer.status = 200;
	sendJ(sock, &buffer);
	if (buffer.status == 500){
		/* TO DISCUSS */
		exit(0);
	}
	printf("Vous êtes actuellement en attente d'une réponse du serveur...\n");
	/* Future interactions with the serveur */
	while(1) {
		readJ(sock, &buffer);
		printf("%s\n", buffer.content);
		fflush(stdin);
		if(buffer.status == 201) {
			break;
		}
	}
	close(sock);
}

/*
 * Used to readJ an input from the keyboard.
 */
void keyboardReader(char** name){
	if(((*name) = (char *) malloc(sizeof(char) * NAME_LENGTH)) == NULL) {
		perror("malloc()");
		exit(errno);
	}
	if ((fgets(*(name), NAME_LENGTH, stdin)) == NULL){
		fprintf(stderr, "fgets problem");
		exit(EXIT_FAILURE);
	}
	if((*name)[strlen(*name)-1] != '\n'){
		perror("Trop grande ligne lue\n");
		while((*name)[strlen(*name) -1] != '\n'){
			fgets(*(name), NAME_LENGTH, stdin);
			exit(11);
		}
	}
	fflush(stdin);
}

/*
 * Receive a message from the specified socket.
 * Returns number of caracs readJ in case of success.
 * Exit in case of error.
 */
int readJ(SOCKET sock, message *  buffer) {
	int n; /* Number of caracs get by recv */
	n = readSocket(sock, buffer, NULL);
	if (n == 0) {
		printf("Le serveur s'est malheureusement déconnecté\n");
		exit(errno);
	}
	return n;
}

/*
 * sendJ a message to the specified socket.
 * Exit in case of error.
 */
void sendJ(SOCKET sock, message * buffer) {
	sendSocket(sock, buffer, NULL);
}
