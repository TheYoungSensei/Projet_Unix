/*
 * ==================================================================
 *
 * Filename : socket.h
 *
 * Description : Header of the joueur.c file
 *
 * Author : MANIET Antoine "nomLogin", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */


 #ifndef JOUEURH_H
 #define JOUEURH_H


 #endif

 #define closesocket(s) close(s)
 int readSocket(SOCKET sock, message * buffer);
 int sendSocket(SOCKET sock, message * buffer);
 int serverInit(int * sock, SOCKADDR_IN * sin, int port, FILE * file);
 SOCKET acceptSocket(SOCKET sock, SOCKADDR_IN * csin, int * sinsize, message * buffer, int i, FILE * file);
 SOCKET joueurInit(const char * hostname, SOCKADDR_IN * sin, int port);
