/*
 * ==================================================================
 *
 * Filename : global.h
 *
 * Description : Header of the global.c file
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */



#ifndef GLOBALH_H
#define GLOBALH_H


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
#include <time.h>


#define TRUE 1
#define ERROR -1
#define MAX_PLAYER 3
#define ERRNO errno
#define TEMPS_SELECT 3
#define NOMBRE_MANCHE 3
#define NOMBRE_TOUR 0
#define INSCRIPTION_TIME 60

#define SYS(call) ((call) == -1) ? perror(#call ": ERROR"), exit(1) : 0
#define SYSN(call) ((call) == NULL) ? perror(#call ": ERROR"), exit(1) : 0

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

typedef struct message {
	int status;
	char content[1024];
} message;

typedef struct card {
  char color[8];
  int value;
  int id;
  /* Maybe int played; 0 - 1 */
} card;

#endif
