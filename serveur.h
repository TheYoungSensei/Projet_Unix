/*
 * ==================================================================
 *
 * Filename : serveur.h
 *
 * Description : Header of the server.c file
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */

#ifndef SERVEURH_H
#define SERVEURH_H

#include "global.h"
#include "socket.h"
#include "ipc.h"

typedef struct client {
  int pseudoKnown;
  char * pseudo;
  SOCKET sock;
} client;

#endif

void lock();
FILE *openFile(const char * name, const char * mode, FILE * file);
void serverSigaction(struct sigaction *act, struct sigaction *actInt, sigset_t *set);
void sendMsgToPlayers(char* message, int stat, int acceptNbr, struct message buffer, struct client* clients);
void closeSockets(SOCKET *sock, client **clients);
void setHandler(struct sigaction * interrupt, sigset_t *set);
int readS(int position, message *  buffer);
void shuffle(int *array, size_t n);
void giveCards(memory* shm, message* buffer, client* clients);
