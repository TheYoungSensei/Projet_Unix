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

#define NAME_LENGTH 64

#endif

SOCKET joueurInit(const char * hostname, SOCKADDR_IN * sin, int port);
void keyboardReader(char** name);
int readJ(SOCKET sock, message *  buffer);
void sendJ(SOCKET sock, message * buffer);
