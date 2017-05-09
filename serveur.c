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
	if(argc == 3) {
		freopen(*++argv, "a", stderr);
	}
	port = atoi(*++argv);
	if(strcmp(*++argv, "stderr")) {
		fderror = openFile(*argv, "w", NULL);
	}
	/* Lock */
	lock(fderror);
	/* Server's initialisation */
	SYS(serverInit(&sock, &sin, port, fderror));
	sinsize = sizeof csin;
	/* Sigaction's initialisation */
	serverSigaction(&act, &actInt, &set, fderror);
	if((players = (player*) malloc(sizeof(player) * MAX_PLAYER)) == NULL) {
		writeToErr(fderror, "malloc()");
		exit(ERRNO);
	}
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
			writeToErr(fderror, "select()");
			exit(ERRNO);
		} else if (timedout != 0) {
			if (FD_ISSET(sock, &readfds)){
				if (acceptNbr == 0) { /* If known pseudos's number = 1 */
					alarm(30);
				}
				players[acceptNbr].pseudoKnown = 0;
				SYS((players[acceptNbr].sock = acceptSocket(sock, &csin, &sinsize, &buffer, acceptNbr, fderror)));
				acceptNbr++;
			} else {
				for (compteur = 0 ; compteur < acceptNbr; compteur++){
					if(FD_ISSET(players[compteur].sock, &readfds)) {
						SYS((n = readSocket(players[compteur].sock, &buffer, fderror)));
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
								SYS(sendSocket(players[i].sock, &buffer, fderror));
							}
						} else {
							/* Adding the pseudo of a player */
							players[compteur].pseudoKnown = 1;
							if((players[compteur].pseudo = (char *) malloc(sizeof(char) * n)) == NULL) {
								writeToErr(fderror, "malloc()");
								exit(ERRNO);
							}
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
	if(sigprocmask(SIG_UNBLOCK, &set, NULL) == ERROR) {
		writeToErr(fderror, "sigprocmask");
		exit(ERRNO);
	}
	/* Sending a message to all accepted but not known players */
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		if(players[compteur].pseudoKnown == 0) {
			buffer.status = 201;
			strcpy(buffer.content, "Délai d'entrée du pseudo écoulé. Vous ne participez pas à cette partie.");
			SYS(sendSocket(players[compteur].sock, &buffer, fderror));
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
		SYS(sendSocket(players[compteur].sock, &buffer, fderror));
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
void lock(FILE * file) {
	int f_lock;
	/* flock to avoid double server opening */
	if ((f_lock = open("serveur.lock", O_RDWR)) == ERROR){
		writeToErr(file, "open()");
		exit(ERRNO);
	}
	if (flock(f_lock, LOCK_EX | LOCK_NB) == ERROR){
		if( errno == EWOULDBLOCK ){

			writeToErr(file, "The server is already launched");
			exit(errno);
		} else {
			writeToErr(file, "flock");
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
	if((fd = fopen(name, mode)) == NULL) {
		writeToErr(file, "fopen()");
		exit(ERRNO);
	}
	return fd;
}


void serverSigaction(struct sigaction *act, struct sigaction *actInt, sigset_t *set, FILE * fderror) {
	act->sa_handler = timeout;
	act->sa_flags = 0;
	actInt->sa_handler = serverInterrupt;
	actInt->sa_flags = 0;
	if (sigemptyset(&(act->sa_mask)) == ERROR){
		writeToErr(fderror, "sigemptyset()");
		exit(ERRNO);
	}
	if(sigfillset(set) == ERROR) {
		writeToErr(fderror, "sigfillset()");
		exit(ERRNO);
	}
	if(sigdelset(set, SIGALRM) == ERROR){
		writeToErr(fderror, "sigdelset()");
		exit(ERRNO);
	}
	if(sigdelset(set, SIGINT) == ERROR){
		writeToErr(fderror, "sigdelset()");
		exit(ERRNO);
	}
	if(sigprocmask(SIG_BLOCK, set, NULL) == ERROR){
		writeToErr(fderror, "sigprocmask()");
		exit(ERRNO);
	}
	if(sigaction(SIGALRM, act, NULL) == ERROR){
		writeToErr(fderror, "sigaction()");
		exit(ERRNO);
	}
	if(sigaction(SIGINT, actInt, NULL) == ERROR){
		writeToErr(fderror, "sigaction()");
		exit(ERRNO);
	}
}
