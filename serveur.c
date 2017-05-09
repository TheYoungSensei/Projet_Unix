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

#include "global.h"
#include "serveur.h"
#include "socket.h"
#include "sharedMemory.h"

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
	player * players;
	struct timeval tv;
	struct sigaction act, actInt;
	sigset_t set;
	fd_set readfds;
	const char *hostname = "127.0.0.1";
	int notNull, i, sinsize, port, n = 0, acceptNbr = 0, pseudosNbr = 0, compteur, maxFd = sock, timedout, f_lock;
	FILE * fderror = NULL;
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
	SYSN((players = (player*) malloc(sizeof(player) * MAX_PLAYER)));
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
			FD_SET(players[compteur].sock, &readfds);
			if (players[compteur].sock>maxFd) maxFd = players[compteur].sock;
		}
		if((timedout = select(maxFd+1, &readfds, NULL, NULL, &tv)) == ERROR){
			if (errno == EINTR){
				if (serverInt == 1) {
					/* CTRL-C as been caught */
					printf("Fin du programme\n");
					closesocket(sock);
					for(compteur =0; compteur < acceptNbr; compteur++) {
						closesocket(players[compteur].sock);
					}
					exit(0);
				} else if (timeoutInt == 1) {
					/* SIGALRM as been caucght */
					continue;
				}
			}
			perror("select()");
			exit(ERRNO);
		} else if (timedout != 0) {
			if (FD_ISSET(sock, &readfds)){
				if (acceptNbr == 0) { /* If known pseudos's number = 1 */
					alarm(30);
				}
				players[acceptNbr].pseudoKnown = 0;
				SYS((players[acceptNbr].sock = acceptSocket(sock, &csin, &sinsize, &buffer, acceptNbr)));
				acceptNbr++;
			} else {
				for (compteur = 0 ; compteur < acceptNbr; compteur++){
					if(FD_ISSET(players[compteur].sock, &readfds)) {
						SYS((n = readSocket(players[compteur].sock, &buffer)));
						if (n == 0) {
							buffer.status = 200;
							/* notNull -> Allow us to know whether we have to remove the pseudo's number */
							notNull = 0;
							if(players[compteur].pseudoKnown != 0) {
								sprintf(buffer.content, "Déconnexion de : %s", players[compteur].pseudo);
								notNull = 1;
							}
							for(i = compteur; i < acceptNbr-1; i++) {
									players[i] = players[i+1];
							}
							acceptNbr--;
							if(notNull == 0) break;
							pseudosNbr--;
							for(i = 0 ;i < acceptNbr; i++) {
								SYS(sendSocket(players[i].sock, &buffer));
							}
						} else {
							/* Adding the pseudo of a player */
							players[compteur].pseudoKnown = 1;
							SYSN((players[compteur].pseudo = (char *) malloc(sizeof(char) * n)));
							strcpy(players[compteur].pseudo, buffer.content);
							players[compteur].pseudo[strlen(players[compteur].pseudo) - 1] = '\0';
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
		if(players[compteur].pseudoKnown == 0) {
			buffer.status = 201;
			strcpy(buffer.content, "Délai d'entrée du pseudo écoulé. Vous ne participez pas à cette partie.");
			SYS(sendSocket(players[compteur].sock, &buffer));
			SYS(closesocket(players[compteur].sock));
			for(i = compteur; i < acceptNbr-1; i++) {
					players[i] = players[i+1];
			}
			acceptNbr--;
		}
	}
	/* Notifying all users about the game's beginning */
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		buffer.status = 201;
		strcpy(buffer.content, "Lancement de la partie !");
		SYS(sendSocket(players[compteur].sock, &buffer));
	}

/* Future game's handeling */

	/* Closing every socket  */
	SYS(closesocket(sock));
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		SYS(closesocket(players[compteur].sock));
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
