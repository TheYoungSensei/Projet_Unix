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

int getMemory() {
  int shmid;
  SYS((shmid = shmget((key_t) KEY_M, SHMSZ, IPC_CREAT | 0666)));
  return shmid;
}

memory attachMemory(int shmid) {
  memory *shm;
  shm = shmat(shmid, NULL, 0);
  return *shm;
}

char *attachMemoryChar(int shmid) {
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

int lecteurPlayers(semaphore *sem, char **nbLecteur, memory *shm) {
  int i;
  int nbPlayers = getNbPlayers(sem, nbLecteur, shm);
  while(TRUE) {
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) + 1));
    if(atoi((*nbLecteur)) == 1) {
      semDown(sem, 1);
    }
    semUp(sem, 0);
    for(i = 0; i < nbPlayers; i++) {
      printf("%s ayant : %d points\n", shm->players[i].pseudo, shm->players[i].score);
    }
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) - 1));
    if(atoi((*nbLecteur)) == 0){
      semUp(sem, 1);
    }
    semUp(sem, 0);
    return 0;
  }

}

int lecteurCards(semaphore *sem, char **nbLecteur, memory *shm) {
  int i;
  int nbCards = getNbCards(sem, nbLecteur, shm);
  while(TRUE) {
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) + 1));
    if(atoi((*nbLecteur)) == 1) {
      semDown(sem, 1);
    }
    semUp(sem, 0);
    for(i = 0; i < nbCards; i++) {
      printf("%d de %c\n", shm->cards[i].value, shm->cards[i].color);
    }
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) - 1));
    if(atoi((*nbLecteur)) == 0){
      semUp(sem, 1);
    }
    semUp(sem, 0);
    return 0;
  }
}

int getNbPlayers(semaphore *sem, char **nbLecteur, memory *shm) {
  int tmp;
  while(TRUE) {
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) + 1));
    if(atoi((*nbLecteur)) == 1) {
      semDown(sem, 1);
    }
    semUp(sem, 0);
    tmp = shm->nbPlayers;
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) - 1));
    if(atoi((*nbLecteur)) == 0){
      semUp(sem, 1);
    }
    semUp(sem, 0);
    return tmp;
  }
}

int getNbCards(semaphore *sem, char **nbLecteur, memory *shm) {
  int tmp;
  while(TRUE) {
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) + 1));
    if(atoi((*nbLecteur)) == 1) {
      semDown(sem, 1);
    }
    semUp(sem, 0);
    tmp = shm->nbCards;
    semDown(sem, 0);
    sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) - 1));
    if(atoi((*nbLecteur)) == 0){
      semUp(sem, 1);
    }
    semUp(sem, 0);
    return tmp;
  }
}


void addCard(semaphore *sem, char **nbLecteur,  memory *shm, card card) {
  int nbCards = getNbCards(sem, nbLecteur, shm);
  semDown(sem, 1);
  shm->cards[nbCards] = card;
  shm->nbCards = nbCards + 1;
  /*ecrireDonnees();*/
  semUp(sem, 1);
}

void addPlayer(semaphore *sem, char **nbLecteur,  memory *shm, player player) {
  int nbPlayers = getNbPlayers(sem, nbLecteur, shm);
  semDown(sem, 1);
  shm->players[nbPlayers] = player;
  shm->nbPlayers = nbPlayers + 1;
  /*ecrireDonnees();*/
  semUp(sem, 1);
}

void removePlayer(semaphore *sem, char **nbLecteur,  memory *shm, int position) {
  int nbPlayers = getNbPlayers(sem, nbLecteur, shm);
  int i;
  semDown(sem, 1);
  for(i = position; i < nbPlayers - 1; i++) {
      shm->players[i] = shm->players[i + 1];
  }
  shm->nbPlayers = nbPlayers - 1;
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

void initSharedMemory(memory *shm, char ** nbLect, semaphore * sem) {
  int shmid;
  shmid = getMemory();
  *shm = attachMemory(shmid);
  shmid = getMemory();
  *nbLect = attachMemoryChar(shmid);
  *sem = sembufInit();
}
