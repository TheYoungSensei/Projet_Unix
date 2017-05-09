/*
 * ==================================================================
 *
 * Filename : serveur.c
 *
 * Description : Used to manipulate the shared memory.
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */
#include "sharedMemory.h"
#include "global.h"

int getMemory(FILE * fderror) {
  int shmid;
  if((shmid = shmget((key_t) KEY_M, SHMSZ, IPC_CREAT | 0666)) < 0) {
    writeToErr(fderror, "shmget()");
    return -1;
  }
  return shmid;
}

char *attachMemory(int shmid, FILE * fderror) {
  char *shm;
  if((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    writeToErr(fderror, "shmat()");
    return NULL;
  }
  return shm;
}

semaphore sembufInit() {
  semaphore sem
  struct sembuf sop[2];
  int semid;
  sop[0].sem_num = 0; /* mutex */
  sop[0].sem_flg = 0;
  sop[1].sem_num = 1; /* sharedMemory */
  sop[1].sem_flg = 0;
  sem.sop = sop;
  if((semid = semget(KEY_M, 2, IPC_CREAT | 0644)) < 0) {
    writeToErr(fderror, "semget()");
    return NULL;
  }
  sem.semid = semid;
  return sem;
}

int lecteur (semaphore *sem, char *nbLecteur, char *shm) {
  char *tmp;
  while(TRUE) {
    semDown(sem, 0);
    *nbLecteur = itoa(atoi((*nbLecteur)) + 1);
    if(atoi((*nbLecteur)) == 1) {
      semDown(sem, 1);
    }
    semUp(sem, 0);
    tmp = shm;
    for(tmp = shm; *tmp != NULL; tmp++) {
      printf("%s\n", tmp);
    }
    semDown(sem, 0);
    *nbLecteur = itoa(atoi((*nbLecteur)) - 1);
    if(atoi((*nbLecteur)) == 0){
      semUp(sem, 1);
    }
    semUp(sem, 0);
    return 0;
  }

}

void redacteur(semaphore *sem, char *shm, char *ajout, int position) {
  semDown(sem, 1);
  ecrireDonnees();
  semUp(sem, 1);
}

void semUp(semaphore * sem, int type) {
  sem->sop[type].sem_op = 1;
  if((semop(sem->semid, &(sem->sop[type]))) < 0) {
    writeToErr(fderror, "semop()");
    return -1;
  }
}

void semDown(semaphore * sem) {
  sem->sop[type].sem_op = -1;
  if((semop(sem->semid, &(sem->sop[type]))) < 0) {
    writeToErr(fderror, "semop()");
    return -1;
  }
}
