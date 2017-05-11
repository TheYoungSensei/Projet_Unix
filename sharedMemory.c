/*
  ==================================================================
 
  Filename : serveur.c
 
  Description : Used to manipulate the shared memory.
 
  Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 
  ==================================================================
 */
#include "sharedMemory.h"

memory * shm;
char**  nbLecteur;
semaphore* sem;

int getMemory() {
	int shmid;
	SYS((shmid = shmget((key_t) KEY_M, SHMSZ, IPC_CREAT | 0666)));
	return shmid;
}

int getMemoryChar() {
	int shmid;
	SYS((shmid = shmget((key_t) 1417, SHMSZ, IPC_CREAT | 0666)));
	return shmid;
}

memory *attachMemory(int shmid) {
	memory *shm;
	shm = shmat(shmid, NULL, 0);
	return shm;
}

char** attachMemoryChar(int shmid) {
	char ** shm;
	if((shm = shmat(shmid, NULL, 0)) == (char **) -1) {
		perror("shmat()");
		return NULL;
	}
	return shm;
}

semaphore *sembufInit() {
	struct sembuf sop[2];
	int semid;
	SYS((semid = semget(0, 1, IPC_CREAT | 0644)));
	sop[0].sem_num = 0; /* mutex */
	sop[0].sem_flg = 0;
	sem->sop[0] = sop[0];
	sem->semid[0] = semid;
	SYS((semid = semget(1, 1, IPC_CREAT | 0644)));
	sop[1].sem_num = 1; /* sharedMemory */
	sop[1].sem_flg = 0;
	sem->sop[1] = sop[1];
	sem->semid[1] = semid;
	semUp(0);
	semUp(1);
	return sem;
}

int mReader(int type) {
	int i, tmp;
	while(TRUE) {
		semDown(0);
		sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) + 1));
		if(atoi((*nbLecteur)) == 1) {
			semDown(1);
		}
		semUp(0);
			printf("%d\n",tmp = shm->nbPlayers);
			printf("BLABLA%d\n", type);
		switch(type){
		case NB_PLAYERS:
			tmp =shm->nbPlayers;
			break;
		case PLAYERS :
			tmp =shm->nbPlayers;    
			for(i = 0; i < tmp; i++) {
				printf("%s ayant : %d points\n", shm->players[i].pseudo, shm->players[i].score);
			}
			break;
		case NB_CARDS :
			tmp =shm->nbCards;
			break;
		case CARDS :
			tmp =shm->nbCards;
			for(i = 0; i < tmp; i++) {
				printf("%d de %c\n", shm->cards[i].value,shm->cards[i].color);
			}
			break;
		}
			semDown(0);
			sprintf(*nbLecteur, "%d", (atoi((*nbLecteur)) - 1));
			if(atoi((*nbLecteur)) == 0){
				semUp(1);
			}
			semUp(0);
			return tmp;
		}
	}


	void addCard(card card) {
		int nbCards = mReader(NB_CARDS);
		semDown(1);
		shm->cards[nbCards] = card;
		shm->nbCards = nbCards + 1;
		semUp(1);
	}

	void addPlayer(player player) {	
		int nbPlayers = mReader(NB_CARDS);
		semDown(1);
		shm->players[nbPlayers] = player;
		shm->nbPlayers = nbPlayers + 1;
		semUp(1);
	}

	void removePlayer(int position) {
		int nbPlayers =  mReader(NB_CARDS);
		int i;
		semDown(1);
		for(i = position; i < nbPlayers - 1; i++) {
			shm->players[i] = shm->players[i + 1];
		}
		shm->nbPlayers = nbPlayers - 1;
		semUp(1);
	}

	void semUp(int type) {
		sem->sop[type].sem_op = 1;
		SYS((semop(sem->semid[type], sem->sop, 1)));
	}

	void semDown(int type) {
		sem->sop[type].sem_op = -1;
		SYS((semop(sem->semid[type], sem->sop, 1)));
	}

	void initSharedMemory() {
		int shmid;
		shmid = getMemory();
		shm = attachMemory(shmid);
		shmid = getMemoryChar();
		nbLecteur = attachMemoryChar(shmid);
		shm->nbPlayers = 0;
		shm->nbCards = 0;
		sem = sembufInit();
	}




