/*
 * ==================================================================
 *
 * Filename : joueur.h
 *
 * Description : Header of the joueur.c file
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */

#ifndef JOUEURH_H
#define JOUEURH_H

#define NAME_LENGTH 64

#endif

void keyboardReader(char** name);
int readJ(SOCKET sock, message *  buffer);
void sendJ(SOCKET sock, message * buffer);
