/*
 * ==================================================================
 *
 * Filename : global.h
 *
 * Description : Header of the joueur.c file and the server.c file
 * 
 * Author : MANIET Antoine "nomLogin", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */



#ifndef GLOBALH_H
#define GLOBALH_H
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define EXIT_ERROR -1
#endif

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

typedef struct message {
	int status;
	char content[1024];
} message;
