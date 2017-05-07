/*
 * ==================================================================
 *
 * Filename : global.h
 *
 * Description : Header of the global.c file
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


#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>


typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif


void writeToErr(FILE * file, char * message);

typedef struct message {
	int status;
	char content[1024];
} message;
