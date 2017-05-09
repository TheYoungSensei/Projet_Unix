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

typedef struct semaphore {
  struct sembuf sop[2];
  int semid;
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
memory attachMemory(int shmid);
char *attachMemoryChar(int shmid);
semaphore sembufInit();
int lecteurPlayers(semaphore *sem, char **nbLecteur, memory *shm);
int lecteurCards(semaphore *sem, char **nbLecteur, memory *shm);
void redacteur(semaphore *sem, memory shm, memory ajout);
int getNbPlayers(semaphore *sem, char **nbLecteur, memory *shm);
int getNbCards(semaphore *sem, char **nbLecteur, memory *shm);
void addCard(semaphore *sem, char **nbLecteur,  memory *shm, card card);
void addPlayer(semaphore *sem, char **nbLecteur,  memory *shm, player player);
void removePlayer(semaphore *sem, char **nbLecteur,  memory *shm, int position);
void semUp(semaphore * sem, int type);
void semDown(semaphore * sem, int type);
void initSharedMemory(memory *shm, char **nbLect, semaphore * sem);
