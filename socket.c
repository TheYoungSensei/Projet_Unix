/*
 * ==================================================================
 *
 * Filename : socket.c
 *
 * Description : file used to manipulate the sockets.
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */

#include "socket.h"

/*
 * Receive a message from the specified socket.
 * Returns numbers of caracs readed.
 * Exit in case of error.
 */
int readSocket(SOCKET sock, message *  buffer) {
	int n; /* Number of caracs get by recv */
	if((n = recv(sock, buffer, sizeof((*buffer)) - 1, 0)) < ERROR) {
		if(errno == EOF) {
			return ERROR; /* Can be replaced with player.disconnect */
		}
		perror("recv()");
    return -2;
	}
	return n;
	/* May be improved */
}


/*
 * Send a message to the specified socket.
 * Exit in case of error.
 */
int sendSocket(SOCKET sock, message * buffer) {
	SYS(send(sock, buffer, sizeof((*(buffer))) ERROR, 0));
	/* May be improved */
	return 0;
}

/*
 * Used to initialize the server.
 * Exit in case of error.
 */
int serverInit(int * sock, SOCKADDR_IN * sin, int port) {
	/* Server Initialisation */
	*(sock) = socket(AF_INET, SOCK_STREAM, 0);
	SYS(*(sock));
	sin->sin_addr.s_addr = htonl(INADDR_ANY);
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	if(bind(*(sock), (SOCKADDR *) sin, sizeof *(sin)) == ERROR) {
		if( errno == EADDRINUSE ) {
			fprintf(stderr, "The server is already launched\n");
      return ERROR;
		} else {
				perror("bind()");
        return ERROR;
		}
	}
	SYS(listen(*(sock), MAX_PLAYER));
	return 0;
}

/*
 * Used to accept a socket.
 * Return the socket created or exit in case of error.
 */
SOCKET acceptSocket(SOCKET sock, SOCKADDR_IN * csin, int * sinsize, message * buffer, int i) {
	SOCKET csock;
	SYS((csock = accept(sock, (SOCKADDR *) csin, (socklen_t *) sinsize)));
	buffer->status = 200;
	sprintf(buffer->content, "Bienvenue sur le jeu de Papayoo du groupe manietSacre.\nIl y a actuellement %d joueur(s) connecté(s).\n", i + 1);
	sendSocket(csock, buffer); /* TO DO */
	return csock;
}


/*
 * Used to initialize the server.
 */
SOCKET joueurInit(const char * hostname, SOCKADDR_IN * sin, int port) {
	struct hostent *hostinfo = NULL;
	SOCKET sock;
	/* Server Initialisation  */
	SYS((sock = socket(AF_INET, SOCK_STREAM, 0)));
	SYSN((hostinfo = gethostbyname(hostname)));
	sin->sin_addr = *(IN_ADDR *) hostinfo->h_addr;
	sin->sin_port = htons(port);
	sin->sin_family = AF_INET;
	return sock;
}

