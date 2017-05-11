/*
 * ==================================================================
 *
 * Filename : serveur.c
 *
 * Description : Header of the sharedMemory.c file.
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */

#ifndef SHM_H
#define SHM_H

#include "global.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHMSZ 666
#define KEY_M 1418
#define PLAYERS 1
#define CARDS 2
#define NB_PLAYERS 3
#define NB_CARDS 4

typedef struct semaphore {
  struct sembuf sop[2];
  int semid[2];
} semaphore;

typedef struct player {
	char *pseudo;
	int score;
	int position; /* TO TEST */
} player;

typedef struct memory {
  int nbPlayers;
  int nbCards;
  card cards[60]; /* Could and should be replaced by a card structure */
  player players[MAX_PLAYER];
} memory;

#endif
int getMemory();
int getMemoryChar();
memory *attachMemory(int shmid);
char **attachMemoryChar(int shmid);
semaphore *sembufInit();
int mReader(int type);
void addCard(card card);
void addPlayer(player player);
void removePlayer(int position);
void semUp(int type);
void semDown(int type);
void initSharedMemory();

