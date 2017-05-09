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

int getMemory() {
  int shmid;
  SYS((shmid = shmget((key_t) KEY_M, SHMSZ, IPC_CREAT | 0666)));
  return shmid;
}

char *attachMemory(int shmid) {
  char *shm;
  if((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat()");
    return NULL;
  }
  return shm;
}

semaphore sembufInit() {
  semaphore sem;
  struct sembuf sop[2];
  int semid;
  sop[0].sem_num = 0; /* mutex */
  sop[0].sem_flg = 0;
  sop[1].sem_num = 1; /* sharedMemory */
  sop[1].sem_flg = 0;
  sem.sop[0] = sop[0];
  sem.sop[1] = sop[1];
  SYS((semid = semget(KEY_M, 2, IPC_CREAT | 0644)));
  sem.semid = semid;
  return sem;
}

int lecteur (semaphore *sem, char **nbLecteur, char **shm) {
  char *tmp;
  while(TRUE) {
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) + 1));
    if(atoi((*nbLecteur)) == 1) {
      semDown(sem, 1);
    }
    semUp(sem, 0);
    /*for(tmp = (*shm); (*tmp) != NULL; tmp++) {
      printf("%s\n", tmp);
    }*/
    semDown(sem, 0);
      sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) - 1));
    if(atoi((*nbLecteur)) == 0){
      semUp(sem, 1);
    }
    semUp(sem, 0);
    return 0;
  }

}

void redacteur(semaphore *sem, char *shm, char *ajout, int position) {
  semDown(sem, 1);
  /*ecrireDonnees();*/
  semUp(sem, 1);
}

void semUp(semaphore * sem, int type) {
  sem->sop[type].sem_op = 1;
  SYS((semop(sem->semid, &(sem->sop[type]), 1)));
}

void semDown(semaphore * sem, int type) {
  sem->sop[type].sem_op = -1;
  SYS((semop(sem->semid, &(sem->sop[type]), 1)));
}

void initSharedMemory(char ** shm, char ** nbLect, semaphore * sem) {
  int shmid;
  shmid = getMemory();
  *shm = attachMemory(shmid);
  shmid = getMemory();
  *nbLect = attachMemory(shmid);
  *sem = sembufInit();
}
