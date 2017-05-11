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

static int timeoutInt = 0;
static int serverInt = 0;

void timeout(int bla){
	timeoutInt = 1;
}

void serverInterrupt(int sig) {
	/* Server Interrupt CTRL-C */
	serverInt = 1;
}

int main(int argc, char** argv) {
	SOCKET sock;
	SOCKADDR_IN sin, csin;
	message buffer;
	client * clients;
	player player;	
	struct timeval tv;
	struct sigaction act, actInt;
	sigset_t set;
	fd_set readfds;
	const char *hostname = "127.0.0.1";
	int notNull, i, sinsize, port, n = 0, acceptNbr = 0, pseudosNbr = 0, compteur, maxFd = sock, timedout, f_lock;
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
	initSharedMemory();
	/* Sigaction's initialisation */
	serverSigaction(&act, &actInt, &set);
	SYSN((clients = (client*) malloc(sizeof(client) * MAX_PLAYER)));
	/* Parametring registration's select */
	maxFd = sock;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	/* Begin registration's while */
	while(1) {
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
					printf("Fin du programme\n");
					closesocket(sock);
					for(compteur =0; compteur < acceptNbr; compteur++) {
						closesocket(clients[compteur].sock);
					}
					exit(0);
				} else if (timeoutInt == 1) {
					/* SIGALRM as been caught */
					continue;
				}
			}
			perror("select()");
			exit(ERRNO);
		} else if (timedout != 0) {
			if (FD_ISSET(sock, &readfds)){
				if (acceptNbr == 0) { /* If known pseudos's number = 1 */
					alarm(15);//TODO return to 30
				}
				clients[acceptNbr].pseudoKnown = 0;
				SYS((clients[acceptNbr].sock = acceptSocket(sock, &csin, &sinsize, &buffer, acceptNbr)));
				acceptNbr++;
			} else {
				for (compteur = 0 ; compteur < acceptNbr; compteur++){
					if(FD_ISSET(clients[compteur].sock, &readfds)) {
						SYS((n = readSocket(clients[compteur].sock, &buffer)));
						if (n == 0) {
							buffer.status = 200;
							/* notNull -> Allow us to know whether we have to remove the pseudo's number */
							notNull = 0;
							if(clients[compteur].pseudoKnown != 0) {
								sprintf(buffer.content, "Déconnexion de : %s", clients[compteur].pseudo);
								notNull = 1;
							}
							for(i = compteur; i < acceptNbr-1; i++) {
								clients[i] = clients[i+1];
							}
							acceptNbr--;
							if(notNull == 0) break;
							pseudosNbr--;
							for(i = 0 ;i < acceptNbr; i++) {
								SYS(sendSocket(clients[i].sock, &buffer));
							}
						} else {
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
	sendMsgToPlayers( "Lancement de la partie !", 201, acceptNbr, buffer, clients);
	/* Future game's handeling */
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		player.pseudo = clients[compteur].pseudo;
		player.score = 0;
		addPlayer(player);
	}








	/* Closing every socket  */
	SYS(closesocket(sock));
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		SYS(closesocket(clients[compteur].sock));
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


void sendMsgToPlayers(char* message, int stat, int acceptNbr, struct message buffer, struct client* clients){
	int compteur;
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		buffer.status = stat;
		strcpy(buffer.content, message);
		SYS(sendSocket(clients[compteur].sock, &buffer));
	}
}



