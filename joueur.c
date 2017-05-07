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
	char * name;
	const char *hostname;
	int n = 0, port;
	SOCKADDR_IN sin = { 0 };
	if(argc != 3) {
		fprintf(stderr, "joueur <port> <ipHost>\n");
		return EXIT_ERROR;
	}
	port = atoi(*++argv);
	hostname = *++argv;
	sock = joueurInit(hostname, &sin, port);
	/* Trying to connect to the server */
	if(connect(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		perror("connect()");
		exit(errno);
	}
	/* Showing Welcome message */
	readSocket(sock, &buffer);
	printf("%s", buffer.content);
	/* Reading the userName */
	printf("Veuillez entrer votre pseudo : \n");
	fflush(stdin);
	keyboardReader(&name);
	strcpy(buffer.content, name);
	buffer.status = 200;
	sendSocket(sock, &buffer);
	if (buffer.status == 500){
		/* TO DISCUSS */
		exit(0);
	}
	printf("Vous êtes actuellement en attente d'une réponse du serveur...\n");
	/* Future interactions with the serveur */
	while(1) {
		readSocket(sock, &buffer);
		printf("%s\n", buffer.content);
		fflush(stdin);
		if(buffer.status == 201) {
			break;
		}
	}
	closesocket(sock);
}

/*
 * Used to read an input from the keyboard.
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
 * Returns number of caracs read in case of success.
 * Exit in case of error.
 */
int readSocket(SOCKET sock, message *  buffer) {
	int n; /* Number of caracs get by recv */
	if((n = recv(sock, buffer, sizeof((*buffer)) - 1, 0)) == -1) {
		perror("recv()");
		exit(errno);
	} else if (n == 0) {
		printf("Le serveur s'est malheureusement déconnecté\n");
		exit(errno);
	}
	return n;
	/* May be improved */
}

/*
 * Send a message to the specified socket.
 * Exit in case of error.
 */
void sendSocket(SOCKET sock, message * buffer) {
	if(send(sock, buffer, sizeof((*(buffer))) -1, 0) < 0) {
		perror("send()");
		exit(errno);
	}
	/* May be improved */
}

/*
 * Used to initialize the server.
 */
SOCKET joueurInit(const char * hostname, SOCKADDR_IN * sin, int port) {
	struct hostent *hostinfo = NULL;
	SOCKET sock;
	/* Server Initialisation  */
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
	sin->sin_addr = *(IN_ADDR *) hostinfo->h_addr;
	sin->sin_port = htons(port);
	sin->sin_family = AF_INET;
	return sock;
}
