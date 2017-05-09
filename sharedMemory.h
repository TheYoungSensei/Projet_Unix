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


#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHMSZ 666
#define KEY_M 1418

typedef struct semaphore {
  struct sembuf sop[2];
  int semid;
} semaphore;

typedef struct memory {
  char ** cards; /* Could and should be replaced by a card structure */
  int * score;
  char ** names;
} memory;

#endif
int getMemory();
memory attachMemory(int shmid);
char *attachMemoryChar(int shmid);
semaphore sembufInit();
int lecteur (semaphore *sem, char **nbLecteur, memory *shm);
void redacteur(semaphore *sem, memory shm, memory ajout);
void semUp(semaphore * sem, int type);
void semDown(semaphore * sem, int type);
void initSharedMemory(memory *shm, char **nbLect, semaphore * sem);
