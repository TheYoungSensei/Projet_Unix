/*
 * ==================================================================
 *
 * Filename : socket.c
 *
 * Description : file used to manipulate the sockets.
 *
 * Author : MANIET Antoine "amaniet152", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */

#include "socket.h"
#include "global.h"

/*
 * Receive a message from the specified socket.
 * Returns numbers of caracs readed.
 * Exit in case of error.
 */
int readSocket(SOCKET sock, message *  buffer, FILE * file) {
	int n; /* Number of caracs get by recv */
	if((n = recv(sock, buffer, sizeof((*buffer)) - 1, 0)) < ERROR) {
		if(errno == EOF) {
			return ERROR; /* Can be replaced with player.disconnect */
		}
		writeToErr(file, "recv()");
    return -2;
	}
	return n;
	/* May be improved */
}

/*
 * Send a message to the specified socket.
 * Exit in case of error.
 */
int sendSocket(SOCKET sock, message * buffer, FILE * file) {
	if(send(sock, buffer, sizeof((*(buffer))) ERROR, 0) < 0) {
		writeToErr(file, "send()");
    return ERROR;
	}
	/* May be improved */
}

/*
 * Used to initialize the server.
 * Exit in case of error.
 */
int serverInit(int * sock, SOCKADDR_IN * sin, int port, FILE * file) {
	/* Server Initialisation */
	*(sock) = socket(AF_INET, SOCK_STREAM, 0);
	if(*(sock) == ERROR) {
		writeToErr(file, "sock()");
    return ERROR;
	}
	sin->sin_addr.s_addr = htonl(INADDR_ANY);
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	if(bind(*(sock), (SOCKADDR *) sin, sizeof *(sin)) == ERROR) {
		if( errno == EADDRINUSE ) {
			printf("The server is already launched\n");
      return ERROR;
		} else {
		    writeToErr(file, "bind()");
        return ERROR;
		}
	}
	if(listen(*(sock), MAX_PLAYER) == ERROR) {
		writeToErr(file, "listen()");
    return ERROR;
	}
}

/*
 * Used to accept a socket.
 * Return the socket created or exit in case of error.
 */
SOCKET acceptSocket(SOCKET sock, SOCKADDR_IN * csin, int * sinsize, message * buffer, int i, FILE * file) {
	SOCKET csock;
	if((csock = accept(sock, (SOCKADDR *) csin, (socklen_t *) sinsize)) == ERROR) {
		writeToErr(file, "accept()");
    return ERROR;
	}
	buffer->status = 200;
	sprintf(buffer->content, "Bienvenue sur le jeu de Papayoo du groupe manietSacre.\nIl y a actuellement %d joueur(s) connecté(s).\n", i + 1);
	sendSocket(csock, buffer, file);
	return csock;
}


/*
 * Used to initialize the server.
 */
SOCKET joueurInit(const char * hostname, SOCKADDR_IN * sin, int port) {
	struct hostent *hostinfo = NULL;
	SOCKET sock;
	/* Server Initialisation  */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == ERROR) {
		perror("socket()");
		return ERROR;
	}
	hostinfo = gethostbyname(hostname);
	if(hostinfo == NULL) {
		fprintf(stderr, "Unknown host %s.\n", hostname);
		return ERROR;
	}
	sin->sin_addr = *(IN_ADDR *) hostinfo->h_addr;
	sin->sin_port = htons(port);
	sin->sin_family = AF_INET;
	return sock;
}
