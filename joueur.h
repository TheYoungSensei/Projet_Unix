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

#include "global.h"
#include "socket.h"
#include "ipc.h"
#include "AfficherCaracteresCarte.h"

#endif

void keyboardReader(char** name);
int readJ(message *  buffer);
void sendJ(message * buffer);
void setHandler(struct sigaction * interrupt, sigset_t *set);
card createCard(int id);
