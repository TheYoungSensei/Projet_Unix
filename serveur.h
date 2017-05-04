/*
 * ==================================================================
 *
 * Filename : serveur.h
 *
 * Description : Header of the server.c file
 * 
 * Author : MANIET Antoine "nomLogin", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */

#ifndef SERVEURH_H
#define SERVEURH_H

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


#define closesocket(s) close(s)
#define MAX_PLAYER 4


#endif

