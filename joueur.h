/*
 * ==================================================================
 *
 * Filename : joueur.h
 *
 * Description : Header of the joueur.c file
 *
 * Author : MANIET Antoine "nomLogin", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */

#ifndef JOUEURH_H
#define JOUEURH_H
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

#define closesocket(s) close(s)
#define NAME_LENGTH 64

#endif

SOCKET joueurInit(const char * hostname, SOCKADDR_IN * sin, int port);
void keyboardReader(char** name);
int readSocket(SOCKET sock, message * buffer);
void sendSocket(SOCKET sock, message * buffer);
