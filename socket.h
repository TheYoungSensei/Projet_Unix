/*
 * ==================================================================
 *
 * Filename : socket.h
 *
 * Description : Header of the joueur.c file
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */


 #ifndef JOUEURH_H
 #define JOUEURH_H


 #endif

 #define closesocket(s) close(s)
 int readSocket(SOCKET sock, message * buffer, FILE * fderror);
 int sendSocket(SOCKET sock, message * buffer, FILE * fderror);
 int serverInit(int * sock, SOCKADDR_IN * sin, int port, FILE * file);
 SOCKET acceptSocket(SOCKET sock, SOCKADDR_IN * csin, int * sinsize, message * buffer, int i, FILE * file);
 SOCKET joueurInit(const char * hostname, SOCKADDR_IN * sin, int port);
