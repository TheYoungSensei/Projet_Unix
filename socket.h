/*
 * ==================================================================
 *
 * Filename : socket.h
 *
 * Description : Header of the socket.c file
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */


#ifndef JOUEURH_H
#define JOUEURH_H

#include "global.h"

#endif

#define closesocket(s) close(s)
int readSocket(SOCKET sock, message * buffer);
int sendSocket(SOCKET sock, message * buffer);
int serverInit(int * sock, SOCKADDR_IN * sin, int port);
SOCKET acceptSocket(SOCKET sock, SOCKADDR_IN * csin, int * sinsize, message * buffer, int i);
SOCKET joueurInit(const char * hostname, SOCKADDR_IN * sin, int port);

