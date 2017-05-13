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
#include "ipc.h"

int getMemory() {
  int shmid;
  SYS((shmid = shmget((key_t) KEY_M, SHMSZ, IPC_CREAT | 0666)));
  return shmid;
}

int getMemoryInt() {
  int shmid;
  SYS((shmid = shmget((key_t) 6666, SHMSZ, IPC_CREAT | 0666)));
  return shmid;
}

memory *attachMemory(int shmid) {
  memory *shm;
  SYSN(shm = (memory *) malloc(sizeof(memory)));
  shm = shmat(shmid, NULL, 0);
  return shm;
}

int *attachMemoryInt(int shmid) {
  int *shm;
  shm = shmat(shmid, NULL, 0);
  return shm;
}

semaphore *sembufInit() {
  semaphore * sem;
  SYSN(sem = (semaphore*) malloc(sizeof(semaphore)));
  struct sembuf sop[2];
  int semid;
  SYS((semid = semget(0, 1, IPC_CREAT | 0644)));
  sop[0].sem_num = 0; /* mutex */
  sop[0].sem_flg = 0;
  sop[0].sem_op = 0;
  sem->semid[0] = semid;
  SYS((semid = semget(1, 1, IPC_CREAT | 0644)));
  sop[1].sem_num = 0; /* sharedMemory */
  sop[1].sem_flg = 0;
  sem->semid[1] = semid;
  sem->sop[0] = sop[0];
  sem->sop[1] = sop[1];
  SYS(semctl(sem->semid[0], sem->sop[0].sem_num, SETVAL, 1));
  SYS(semctl(sem->semid[1], sem->sop[1].sem_num, SETVAL, 1));
  return sem;
}

int mReader(semaphore **sem, int **nbLecteur, memory **shm, int type) {
  int i, tmp;
  char * pseudo;
  while(TRUE) {
    semDown(sem, 0);
    **nbLecteur = **nbLecteur + 1;
    if((**nbLecteur) == 1) {
      semDown(sem, 1);
    }
    semUp(sem, 0);
    switch(type) {
      case NB_PLAYERS :
        tmp = (*shm)->nbPlayers;
        break;
      case PLAYERS :
        tmp = (*shm)->nbPlayers;
        for(i = 0; i < tmp; i++) {
          pseudo = (*shm)->players[i].pseudo;
          if(pseudo == NULL) {
            printf("NULL\n");
          } else {
            printf("%s a est le joueur n°%d\n", pseudo, i);
          }
        }
        break;
      case NB_CARDS :
        tmp = (*shm)->nbCards;
        break;
      case CARDS :
        tmp = (*shm)->nbCards;
        for(i = 0; i < tmp; i++) {
          printf("%d de %s\n", (*shm)->cards[i].value, (*shm)->cards[i].color);
        }
        break;
    }
    semDown(sem, 0);
    **nbLecteur = **nbLecteur -1;
    if((**nbLecteur) == 0){
      semUp(sem, 1);
    }
    semUp(sem, 0);
    return tmp;
  }

}


void addCard(semaphore **sem, int **nbLecteur,  memory **shm, card card) {
  int nbCards = mReader(sem, nbLecteur, shm, NB_CARDS);
  semDown(sem, 1);
  strcpy((*shm)->cards[nbCards].color, card.color);
  (*shm)->cards[nbCards].value = card.value;
  (*shm)->nbCards = nbCards + 1;
  semUp(sem, 1);
}

void addPlayer(semaphore **sem, int **nbLecteur,  memory **shm, player player) {
  int nbPlayers = mReader(sem, nbLecteur, shm, NB_PLAYERS);
  semDown(sem, 1);
  fflush(stdout);
  strcpy(((*shm)->players[nbPlayers].pseudo), player.pseudo);
  (*shm)->players[nbPlayers].score  = player.score;
  (*shm)->nbPlayers = (*shm)->nbPlayers + 1;
  semUp(sem, 1);
}

void removePlayer(semaphore **sem, int **nbLecteur,  memory **shm, int position) {
  int nbPlayers = mReader(sem, nbLecteur, shm, NB_PLAYERS);
  int i;
  semDown(sem, 1);
  for(i = position; i < (*shm)->nbPlayers - 1; i++) {
      (*shm)->players[i] = (*shm)->players[i + 1];
  }
  (*shm)->nbPlayers = (*shm)->nbPlayers - 1;
  semUp(sem, 1);
}

void semUp(semaphore ** sem, int type) {
  (*sem)->sop[type].sem_op = 1;
  SYS((semop((*sem)->semid[type], &((*sem)->sop[type]), 1)));
}

void semDown(semaphore ** sem, int type) {
  (*sem)->sop[type].sem_op = -1;
  SYS((semop((*sem)->semid[type], &((*sem)->sop[type]), 1)));
}

void initSharedMemory(memory **shm, int ** nbLect, semaphore ** sem) {
  int shmid;
  shmid = getMemory();
  *shm = attachMemory(shmid);
  shmid = getMemoryInt();
  *nbLect = attachMemoryInt(shmid);
  *sem = sembufInit();
  (*shm)->nbPlayers = 0;
  (*shm)->nbPlayers = 0;
  **nbLect = 0;
}

void closeIPCs(memory ** shm, int** nbLect, semaphore ** sem) {
  shmdt(*shm);
  shmdt(*nbLect);
  /*semctl((*sem)->semid[0], (*sem)->sop[0].sem_num, IPC_RMID, NULL);
  semctl((*sem)->semid[1], (*sem)->sop[1].sem_num, IPC_RMID, NULL);*/
}
