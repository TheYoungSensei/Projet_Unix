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
	if((n = recv(sock, buffer, sizeof((*buffer)) - 1, 0)) < -1) {
		if(errno == EOF) {
			return -1; /* Can be replaced with player.disconnect */
		}
		writeToErr(file, "recv()");
	}
	return n;
	/* May be improved */
}

/*
 * Send a message to the specified socket.
 * Exit in case of error.
 */
void sendSocket(SOCKET sock, message * buffer, FILE * file) {
	if(send(sock, buffer, sizeof((*(buffer))) -1, 0) < 0) {
		writeToErr(file, "send()");
	}
	/* May be improved */
}
