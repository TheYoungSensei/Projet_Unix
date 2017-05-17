/*
 * ==================================================================
 *
 * Filename : serveur.c
 *
 * Description : file used to play the server role.
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */

#include "serveur.h"

/* Needed outside of the main and more precisely in signals management */
int serverInt = 0, acceptNbr = 0, timeoutInt = 0,  pseudosNbr = 0, sinsize;
SOCKET sock;
client * clients;
SOCKADDR_IN sinSock, csinSock;
memory * shm;
int * nbLect;
semaphore * sem;

/*
 * Used to catch SIGINT, SIGQUIT and SIGTERM signals during the game.
 */
void interruptHandler(int sigint) {
	closeAllIPCs(&shm, &nbLect, &sem);
	closeSockets(&sock, &clients);
	exit(0);
}

/*
 * Used to catch the SIGALRM signal during the login (After INSCRIPTION_TIME seconds).
 */
void timeout(int bla){
	timeoutInt = 1;
}

/*
 * Used to catch the SIGINT signal during the login
 */
void serverInterrupt(int sig) {
	/* Server Interrupt CTRL-C */
	serverInt = 1;
	/* CTRL-C as been caught */
	closeSockets(&sock, &clients);
	closeAllIPCs(&shm, &nbLect, &sem);
	exit(0);
}

int main(int argc, char** argv) {
	srand(time(NULL));
	message buffer;
	message * allMessages;
	player player;
	int trash;
	char * charBuf;
	char * charBuf2;
	char payoo[8];
	int totalScore = 0;
	int winner = 0;
	int maxValue = 0;
	char color[8] = "";
	struct timeval tv;
	struct sigaction act, actInt, interrupt;
	sigset_t set;
	fd_set readfds;
	const char *hostname = "127.0.0.1";
	int notNull, i, port, n = 0, manche = 0, tour = 0, compteur, maxFd = sock, timedout, f_lock;
	/* Arguments management */
	if(argc != 3 && argc != 2) {
		fprintf(stderr, "serveur <numPort> <stderr>\n");
		return ERROR;
	}
	port = atoi(*++argv);
	if(argc == 3) {
		freopen(*++argv, "a", stderr);
	}
	/* Lock */
	lock();
	/* Server's initialisation */
	SYS(serverInit(&sock, &sinSock, port));
	while(1) { /* Server is Running */
		act = emptySig;
		actInt = emptySig;
		interrupt = emptySig;
		totalScore = 0;
		winner = 0;
		maxValue = 0;
		notNull = 0;
		i = 0;
		sinsize = 0;
		port = 0;
		n = 0;
		manche = 0;
		tour = 0;
		compteur = 0;
		timedout = 0;
		sinsize = sizeof csinSock;
		serverInt = 0;
		acceptNbr = 0;
		timeoutInt = 0;
		pseudosNbr = 0;
		/* Sigaction's initialisation */
		serverSigaction(&act, &actInt, &set);
		initSharedMemory(&shm, &nbLect, &sem);
		/* Parametring registration's select */
		SYSN((clients = (client*) malloc(sizeof(client) * MAX_PLAYER)));
		SYSN((charBuf) = (char *) malloc(sizeof(char) * 256));
		SYSN((charBuf2) = (char *) malloc(sizeof(char) * 256));
		maxFd = sock;
		/* Begin registration's while */
		while(1) {
			tv.tv_sec = TEMPS_SELECT;
			tv.tv_usec = 0;
			usleep(50);
			/* After 30 sec if there are 2 players or more */
			if(timeoutInt == 1 && pseudosNbr > 1) {
				printf("La partie va commencer.\n");
				break;
			}
			/* Parametring registration's select */
			FD_ZERO(&readfds);
			FD_SET(sock, &readfds);
			/* Add every player to the select */
			for (compteur = 0 ; compteur < acceptNbr; compteur++){
				FD_SET(clients[compteur].sock, &readfds);
				if (clients[compteur].sock>maxFd) maxFd = clients[compteur].sock;
			}
			if((timedout = select(maxFd+1, &readfds, NULL, NULL, &tv)) == ERROR){
				if (errno == EINTR){
					if (serverInt == 1) {
						/* CTRL-C as been caught */
						closeSockets(&sock, &clients);
						closeAllIPCs(&shm, &nbLect, &sem);
						exit(0);
					} else if (timeoutInt == 1) {
						/* SIGALRM as been caught */
						continue;
					}
				}
				perror("select()");
				exit(ERRNO);
			} else {
				if (FD_ISSET(sock, &readfds)){
					if (acceptNbr == 0) { /* If known pseudos's number = 1 */
						SYS(alarm(INSCRIPTION_TIME));
					}
					if(acceptNbr == MAX_PLAYER) {
						acceptSocket(sock, &csinSock, &sinsize, &buffer, acceptNbr);
					} else {
						clients[acceptNbr].pseudoKnown = 0;
						SYS((clients[acceptNbr].sock = acceptSocket(sock, &csinSock, &sinsize, &buffer, acceptNbr)));
						acceptNbr++;
					}
				} else {
					for (compteur = 0 ; compteur < acceptNbr; compteur++){
						if(FD_ISSET(clients[compteur].sock, &readfds)) {
							SYS(n = readS(compteur, &buffer));
							if(n != 0) {
								/* Adding the pseudo of a player */
								clients[compteur].pseudoKnown = 1;
								SYSN((clients[compteur].pseudo = (char *) malloc(sizeof(char) * n)));
								strcpy(clients[compteur].pseudo, buffer.content);
								clients[compteur].pseudo[strlen(clients[compteur].pseudo) - 1] = '\0';
								pseudosNbr++;
							}
						}
					}
				}
			}
		}
		/* Unblocking signals */
		SYS(sigprocmask(SIG_UNBLOCK, &set, NULL));
		setHandler(&interrupt, &set);

		/* malloc allMessages */
		SYSN(allMessages = (struct message*) malloc(sizeof(struct message)*acceptNbr));

		/* Sending a message to all accepted but not known players */
		for(compteur = 0; compteur < acceptNbr; compteur++) {
			if(clients[compteur].pseudoKnown == 0) {
				buffer.status = 201;
				strcpy(buffer.content, "Délai d'entrée du pseu ecoule. Vous ne participez pas a cette partie.");
				SYS(sendSocket(clients[compteur].sock, &buffer));
				SYS(closesocket(clients[compteur].sock));
				for(i = compteur; i < acceptNbr-1; i++) {
					clients[i] = clients[i+1];
				}
				acceptNbr--;
			}
		}
		/* Notifying all users about the game's beginning */
		sendMsgToPlayers("Lancement de la partie !\nVoici le placement des joueurs (2 est a gauche de 1 etc) : \n", 201, acceptNbr, buffer, clients);
		/* Adding the players into the sharedMemory */
		for(compteur = 0; compteur < acceptNbr; compteur++) {
			strcpy(player.pseudo, clients[compteur].pseudo);
			player.score = 0;
			addPlayer(&sem, &nbLect, &shm, player);
		}

		/* Start of the "manches" */
		while (manche < NOMBRE_MANCHE){
			/* Choosing payoo */
			n = rand()%4;
			switch (n){
			case 0:
				strcpy(payoo, COEUR);
				break;
			case 1:
				strcpy(payoo, CARREAU);
				break;
			case 2:
				strcpy(payoo, TREFLE);
				break;
			case 3:
				strcpy(payoo, PIQUE);
				break;
			}
			sprintf(charBuf, "%s\n", payoo);
			sendMsgToPlayers(charBuf, 202, acceptNbr, buffer, clients);

			/* Giving cards to players */
			giveCards(shm, &buffer, clients);
			shm->nbCards = 0;
			/* Retrieving removed cards */
			for(compteur = 0; compteur < acceptNbr; compteur++) {
				readS(compteur, &buffer);
				strcpy(allMessages[compteur].content, buffer.content);
			}

			/* Sending removed cards to players on their left ((compteur+1)%acceptNbr). allMessages -> Because we need to end the withdraws before sending cards back */
			for(compteur = 0; compteur < acceptNbr; compteur++) {
				allMessages[compteur].status = 203;
				SYS(sendSocket(clients[(compteur+1)%acceptNbr].sock, &(allMessages[compteur])));
			}

			/* Player number (manche+tour)%acceptNbr is beginning the "manche" */
			strcpy(charBuf, "a toi de jouer ! Cartes déposées dans le tour actuel : _");
			card cards[acceptNbr];
			/* TODO 6 -> 60 */
			/* TODO @ManietAntoine */
			while (shm->nbCards < NOMBRE_TOUR * acceptNbr) {
				buffer.status = 204;
				strcpy(buffer.content, charBuf);
				SYS(sendSocket(clients[(acceptNbr+winner+manche+tour)%acceptNbr].sock, &buffer));
				readS((acceptNbr+winner+manche+tour)%acceptNbr, &buffer);
				strcat(charBuf, buffer.content);
				strcat(charBuf,"_");
				cards[(manche+tour)%acceptNbr] = createCard(atoi(buffer.content));
				addCard(&sem,&nbLect, &shm, atoi(buffer.content));
				tour++;
				if (tour%acceptNbr==0){
					/* Fin de tour. Traitement des cartes jouées, message de fin de tour aux joueurs, édition du score */
					for (n=0;n<acceptNbr;n++){
						printf("%d\n",(manche+tour)%acceptNbr+n);
						if (n==0){
							strcpy(color,cards[(manche+tour+n)%acceptNbr].color);
							maxValue = cards[(manche+tour+n)%acceptNbr].value;
							printf("%d\n",maxValue);
							winner = (manche+tour)%acceptNbr+n;
						}
						if (cards[(manche+tour)%acceptNbr+n].value == 7 && !strcmp(cards[((manche+tour)+n)%acceptNbr].color, payoo)){
							totalScore = totalScore+40;
						}
						if (!strcmp(cards[((manche+tour)+n)%acceptNbr].color, "Papayoo")){
							totalScore = totalScore+cards[((manche+tour)+n)%acceptNbr].value;
						}

						if (!strcmp(cards[((manche+tour)+n)%acceptNbr].color, color)){
							if (maxValue < cards[((manche+tour)+n)%acceptNbr].value){
								maxValue = cards[((manche+tour)+n)%acceptNbr].value;
								winner = ((manche+tour)+n)%acceptNbr;
							} else {
							}
						}
					}
					sprintf(charBuf, "Résultat du tour : \nLe joueur '%s' perd ce tour.\nIl remporte un score de %d.\n", shm->players[winner].pseudo,totalScore );
					sendMsgToPlayers(charBuf, 205, acceptNbr, buffer, clients);
					shm->players[winner].score =  shm->players[winner].score + totalScore;
					totalScore = 0;
					strcpy(charBuf, "À toi de jouer ! Cartes déposées dans le tour actuel : _");
				}
			}
			/* Message de fin de manche. Rappel des points actuels aux joueurs. Empty cards in SHM */
			strcpy(charBuf, "Résultat de la manche. Les scores des joueurs sont les suivants : \n");
			for (n=0;n<acceptNbr;n++){
				sprintf(charBuf2,"%s : %d.\n", shm->players[n].pseudo, shm->players[n].score);
				strcat(charBuf, charBuf2);
			}
			sendMsgToPlayers(charBuf, 206, acceptNbr, buffer, clients);
			for (n=0;n<60;n++){
				shm->cards[n].id = -1;
			}
			manche++;

		}
		/* Message de fin de partie. Gagnant déterminé. Message d'au revoir. */
		strcpy(charBuf, "");
		n = 250;
		for (compteur=0;compteur<acceptNbr;compteur++){
			if (shm->players[compteur].score < n){
				n = shm->players[compteur].score;
				strcpy(charBuf, "");
				sprintf(charBuf, "Le grand gagnant est '%s' avec un score de seulement %d\n",  shm->players[compteur].pseudo, shm->players[compteur].score);
			}
		}
		sendMsgToPlayers(charBuf, 210, acceptNbr, buffer, clients);
		sendMsgToPlayers("Merci d'avoir joué à notre version du Papayoo. Bonne journée !\n", 600, acceptNbr, buffer, clients);
		closeAllIPCs(&shm, &nbLect, &sem);
		SYS(sigprocmask(SIG_UNBLOCK, &set, NULL));
	}
	closeSockets(&sock, &clients);
	exit(0);
}

/*
 * Used to create a card from it's id.
 */
card createCard(int id){
	card toReturn;
	toReturn.id = id;
	switch(id){
	case 0 ... 9 :
	strcpy(toReturn.color,COEUR);
	toReturn.value = id-0+1;
	break;
	case 10 ... 19:
	strcpy(toReturn.color,CARREAU);
	toReturn.value = id-10+1;
	break;
	case 20 ... 29:
	strcpy(toReturn.color,TREFLE);
	toReturn.value = id-20+1;
	break;
	case 30 ... 39:
	strcpy(toReturn.color,PIQUE);
	toReturn.value = id-30+1;
	break;
	case 40 ... 59:
	strcpy(toReturn.color,"Papayoo");
	toReturn.value = id-40+1;
	break;
	}
	return toReturn;
}


/*
 * Used to give cards to players.
 */

void giveCards(memory* shm, message* buffer, client* clients){
	int i, compteur, compteur2, nbP = shm->nbPlayers;
	int *list;
	list  = (int *) malloc(MAXCARDS * sizeof(int));
	for(i=0; i<MAXCARDS; i++){
		list[i] = i;
	}
	shuffle(list, MAXCARDS);
	for(compteur = 0; compteur < nbP; compteur++) {
		buffer->status = 202;
		for (compteur2=0;compteur2<MAXCARDS/nbP;compteur2++){
			sprintf(buffer->content,"%d",list[compteur2+compteur*MAXCARDS/nbP]);
			SYS(sendSocket(clients[compteur].sock, buffer));
		}
		buffer->status = 500;
		SYS(sendSocket(clients[compteur].sock, buffer));
	}
}

/*
 * Used to randomize the order of cards.
 */
void shuffle(int *array, size_t n){
	if (n > 1)
	{
		size_t i;
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			int t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

/*
 * Used to make the server reacting like a singleton.
 * Exit in case of error.
 */
void lock() {
	int f_lock;
	/* flock to avoid double server opening */
	SYS((f_lock = open("serveur.lock", O_RDWR)));
	if (flock(f_lock, LOCK_EX | LOCK_NB) == ERROR){
		if( errno == EWOULDBLOCK ){
			fprintf(stderr, "Le serveur est deja en cours dexecution\n");
			exit(errno);
		} else {
			perror("flock()");
			exit(ERRNO);
		}
	}
}

/*
 * Used to open a file in the specified mode.
 * Return the FILE * created or exit in case of error.
 */
FILE *openFile(const char * name, const char * mode, FILE * file) {
	FILE *fd;
	SYSN((fd = fopen(name, mode)));
	return fd;
}


/*
 *  Used to initiate the sigactions to handle signals SIGINT, SIGALRM during the login.
 */
void serverSigaction(struct sigaction *act, struct sigaction *actInt, sigset_t *set) {
	act->sa_handler = timeout;
	act->sa_flags = 0;
	actInt->sa_handler = serverInterrupt;
	actInt->sa_flags = 0;
	SYS(sigemptyset(&(act->sa_mask)));
	SYS(sigfillset(set));
	SYS(sigdelset(set, SIGALRM));
	SYS(sigdelset(set, SIGINT));
	SYS(sigprocmask(SIG_BLOCK, set, NULL));
	SYS(sigaction(SIGALRM, act, NULL));
	SYS(sigaction(SIGINT, actInt, NULL));
}
/*
 * Used to send a message to all players.
 */
void sendMsgToPlayers(char* message, int stat, int acceptNbr, struct message buffer, struct client* clients) {
	int compteur;
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		buffer.status = stat;
		strcpy(buffer.content, message);
		SYS(sendSocket(clients[compteur].sock, &buffer));
	}
}
/*
 *  Used to initiate the sigactions to handle signals SIGINT, SIGQUIT and SIGTERM during the game.
 */
void setHandler(struct sigaction * interrupt, sigset_t *set) {
	interrupt->sa_handler = interruptHandler;
	interrupt->sa_flags = 0;
	SYS(sigemptyset(&(interrupt->sa_mask)));
	SYS(sigfillset(set));
	SYS(sigdelset(set, SIGTERM));
	SYS(sigdelset(set, SIGINT));
	SYS(sigdelset(set, SIGQUIT));
	SYS(sigprocmask(SIG_BLOCK, set, NULL));
	SYS(sigaction(SIGTERM, interrupt, NULL));
	SYS(sigaction(SIGINT, interrupt, NULL));
	SYS(sigaction(SIGQUIT, interrupt, NULL));
}
/*
 * Used to close the server's socket and the client's sockets.
 */
void closeSockets(SOCKET *sock, client **clients) {
	int compteur;
	printf("Fin du programme\n");
	closesocket(*sock);
	for(compteur =0; compteur < acceptNbr; compteur++) {
		closesocket((*clients)[compteur].sock);
	}
}
/*
 * Receive a message from the specified socket.
 * Returns number of caracs readed in case of success.
 * Exit in case of error.
 */
int readS(int position, message  * buffer) {
	int n, notNull, i, maxFd, timedout; /* Number of caracs get by recv */
	struct timeval tv;
	fd_set readfds;
	SOCKET csock;
	while(1) {
		tv.tv_sec = 60;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		maxFd = sock;
		FD_SET(clients[position].sock, &readfds);
		if(sock < clients[position].sock) maxFd = clients[position].sock;
		if((timedout = select(maxFd + 1, &readfds, NULL, NULL, &tv)) == -1) {
			perror("select()");
			exit(ERRNO);
		}
		if(FD_ISSET(sock, &readfds)) {
			/* Game is launched so this a to late connexion */
			SYS((csock = accept(sock, (SOCKADDR *) &csinSock, (socklen_t *) &sinsize)));
			sprintf(buffer->content, "La partie a déjà débutée.\n");
			buffer->status = 500;
			sendSocket(csock, buffer);
			continue;
		} else {
			n = readSocket(clients[position].sock, buffer);
			if (n == 0) {
				SYS(closesocket(clients[position].sock));
				buffer->status = 200;
				/* notNull -> Allow us to know whether we have to remove the pseudo's number */
				notNull = 0;
				if(clients[position].pseudoKnown != 0) {
					sprintf(buffer->content, "Déconnexion de : %s", clients[position].pseudo);
					printf("%s\n", buffer->content);
					notNull = 1;
				}
				for(i = position; i < acceptNbr-1; i++) {
					clients[i] = clients[i+1];
				}
				acceptNbr--;
				if(notNull == 0) return n;
				pseudosNbr--;
				for(i = 0 ; i < acceptNbr; i++) {
					SYS(sendSocket(clients[i].sock, buffer));
				}
				printf("Fin de la partie\n");
				buffer->status = 600;
				strcpy(buffer->content, "Il n'y a plus assez de joueurs disponible\nFin de la partie\n");
				for(i = 0; i < acceptNbr; i++) {
					sendSocket(clients[i].sock, buffer);
				}
				closeAllIPCs(&shm, &nbLect, &sem);
				closeSockets(&sock ,&clients);
				exit(25);
			}
			return n;
		}
	}
}
