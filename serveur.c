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
int serverInt = 0, acceptNbr = 0, timeoutInt = 0,  pseudosNbr = 0;
SOCKET sock;
client * clients;
memory * shm;
int * nbLect;
semaphore * sem;

/*
 * Used to catch SIGINT, SIGQUIT and SIGTERM signals during the game.
 */
void interruptHandler(int sigint) {
	printf("Interruption du serveur : %d\n", sigint);
	closeIPCs(&shm, &nbLect, &sem);
	closeSockets(&sock, &clients);
	exit(0);
}

/*
 * Used to catch the SIGALRM signal during the login (After 30 seconds).
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
}

int main(int argc, char** argv) {
	srand(time(NULL));
	SOCKADDR_IN sin, csin;
	message buffer;
	message * allMessages;
	player player;
	struct timeval tv;
	struct sigaction act, actInt, interrupt;
	sigset_t set;
	fd_set readfds;
	const char *hostname = "127.0.0.1";
	int notNull, i, sinsize, port, n = 0, compteur, maxFd = sock, timedout, f_lock;
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
	SYS(serverInit(&sock, &sin, port));
	sinsize = sizeof csin;
	/* Sigaction's initialisation */
	serverSigaction(&act, &actInt, &set);
	initSharedMemory(&shm, &nbLect, &sem);
	SYSN((clients = (client*) malloc(sizeof(client) * MAX_PLAYER)));
	/* Parametring registration's select */
	maxFd = sock;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	/* Begin registration's while */
	while(1) {
		usleep(50);
		/* After 30 sec if there are 2 players or more */
		if(timeoutInt == 1 && pseudosNbr > 1) {
			printf("La partie va commencer\n");
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
					closeIPCs(&shm, &nbLect, &sem);
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
					alarm(5); //TODO return to 30
				}
				clients[acceptNbr].pseudoKnown = 0;
				SYS((clients[acceptNbr].sock = acceptSocket(sock, &csin, &sinsize, &buffer, acceptNbr)));
				acceptNbr++;
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
			strcpy(buffer.content, "Délai d'entrée du pseudo écoulé. Vous ne participez pas à cette partie.");
			SYS(sendSocket(clients[compteur].sock, &buffer));
			SYS(closesocket(clients[compteur].sock));
			for(i = compteur; i < acceptNbr-1; i++) {
					clients[i] = clients[i+1];
			}
			acceptNbr--;
		}
	}
	/* Notifying all users about the game's beginning */
	sendMsgToPlayers("Lancement de la partie !\nVoici le placement des joueurs (2 est à gauche de 1 etc) : ", 201, acceptNbr, buffer, clients);
	/* Adding the players into the sharedMemory */
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		strcpy(player.pseudo, clients[compteur].pseudo);
		player.score = 0;
		addPlayer(&sem, &nbLect, &shm, player);
	}
	/* Giving cards to players */
	giveCards(shm, &buffer, clients);
	/* Retrieving removed cards */
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		readS(compteur, &buffer);
		strcpy(allMessages[compteur].content, buffer.content);
	}

	/* Sending removed cards to players on their left ((compteur+1)%acceptNbr) */
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		allMessages[compteur].status = 203;
		SYS(sendSocket(clients[(compteur+1)%acceptNbr].sock, &(allMessages[compteur])));
	}


	/* Future game's handeling */


	/* Closing every socket  */
	SYS(closesocket(sock));
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		SYS(closesocket(clients[compteur].sock));
	}
	closeSockets(&sock, &clients);
	closeIPCs(&shm, &nbLect, &sem);
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
			fprintf(stderr, "The server is already launched");
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
	int n, notNull, i; /* Number of caracs get by recv */
	n = readSocket(clients[position].sock, buffer);
	if (n == 0) {
		SYS(closesocket(clients[position].sock));
		buffer->status = 200;
		/* notNull -> Allow us to know whether we have to remove the pseudo's number */
		notNull = 0;
		if(clients[position].pseudoKnown != 0) {
			sprintf(buffer->content, "Déconnexion de : %s", clients[position].pseudo);
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
	}
	return n;
}
