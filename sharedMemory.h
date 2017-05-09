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

typedef struct semaphore {
  struct sembuf sop[2];
  int semid;
}semaphore;

#define SHMSZ 666
#define KEY_M 1418

#endif
int getMemory(FILE * fderror);
char *attachMemory(int shmid, FILE * fderror);
semaphore sembufInit();
int lecteur (semaphore *sem, char *nbLecteur, char *shm);
void redacteur(semaphore *sem, char *shm, char *ajout, int position);
void semUp(semaphore * sem, int type);
void semDown(semaphore * sem);
