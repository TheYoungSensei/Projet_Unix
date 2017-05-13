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
#define NAME_LENGTH 64
#define MAXCARDS 60


typedef struct semaphore {
  struct sembuf sop[2];
  int semid[2];
} semaphore;

typedef struct player {
	char pseudo[NAME_LENGTH];
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
int getMemoryInt();
memory *attachMemory(int shmid);
int *attachMemoryInt(int shmid);
semaphore *sembufInit();
int mReader(semaphore **sem, int **nbLecteur, memory **shm, int type);
void addCard(semaphore **sem, int **nbLecteur,  memory **shm, card card);
void addPlayer(semaphore **sem, int **nbLecteur,  memory **shm, player player);
void removePlayer(semaphore **sem, int **nbLecteur,  memory **shm, int position);
void semUp(semaphore ** sem, int type);
void semDown(semaphore ** sem, int type);
void initSharedMemory(memory **shm, int **nbLect, semaphore **sem);
void closeIPCs(memory ** shm, int** nbLect, semaphore ** sem);
