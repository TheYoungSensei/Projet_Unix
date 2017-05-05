/*
 * ==================================================================
 *
 * Filename : serveur.c
 *
 * Description : file used to play the server role.
 *
 * Author : MANIET Antoine "amaniet152", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */

#include "global.h"
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
	SOCKET csock[MAX_PLAYER];
	char * pseudos[MAX_PLAYER];
	struct timeval tv;
	struct sigaction act, actInt;
	sigset_t set;
	fd_set readfds;
	int fd_stdin = fileno(stdin);
	pid_t cpid;
	const char *hostname = "127.0.0.1";
	int i, sinsize, port, n = 0, acceptNbr = 0, pseudosNbr = 0, compteur, maxFd = sock, timedout, f_lock;
	FILE * fderror = NULL;
	/* Arguments  Management */
	if(argc != 3) {
		fprintf(stderr, "serveur [numPort] [stderr]\n");
		return EXIT_ERROR;
	}
	port = atoi(*++argv);
	if(strcmp(*++argv, "stderr")) {
		fderror = openFile(*argv, "w", NULL);
	}
	/* Lock */
	lock(fderror);
	/* Server Initialisation */
	serverInit(&sock, &sin, port, fderror);
	sinsize = sizeof csin;
	/* Sigaction Initialisation */
	act.sa_handler = timeout;
	act.sa_flags = 0;
	actInt.sa_handler = serverInterrupt;
	actInt.sa_flags = 0;
	if (sigemptyset(&act.sa_mask) == -1){
		writeToErr(fderror, "sigemptyset()");
	}
	if(sigfillset(&set) == -1) {
		writeToErr(fderror, "sigfillset()");
	}
	if(sigdelset(&set, SIGALRM) == -1){
		writeToErr(fderror, "sigdelset()");
	}
	if(sigdelset(&set, SIGINT) == -1){
		writeToErr(fderror, "sigdelset()");
	}
	if(sigprocmask(SIG_BLOCK, &set, NULL) == -1){
		writeToErr(fderror, "sigprocmask()");
	}
	if(sigaction(SIGALRM, &act, NULL) == -1){
		writeToErr(fderror, "sigaction()");
	}
	if(sigaction(SIGINT, &actInt, NULL) == -1){
		writeToErr(fderror, "sigaction()");
	}
	/* Set Max Time for Select */
	/* Begin Inscription */
	while(1) {
		if(timeoutInt == 1 && pseudosNbr > 1) {
			printf("La partie va commencer\n");
			break;
		}
		printf("pseudos : %d\n", pseudosNbr);
		printf("time : %d\n", timeoutInt);
		maxFd = sock;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		for (compteur = 0 ; compteur < acceptNbr; compteur++){
			FD_SET(csock[compteur], &readfds);
			if (csock[compteur]>maxFd) maxFd = csock[compteur];
		}
		if((timedout = select(maxFd+1, &readfds, NULL, NULL, &tv)) == -1){
			if (errno == EINTR){
				if (serverInt == 1) {
					printf("Fin du programme\n");
					closesocket(sock);
					for(compteur =0; compteur < acceptNbr; compteur++) {
						closesocket(csock[compteur]);
					}
					exit(0);
				} else if (timeoutInt == 1) {
					//IGNORE
					continue;
				}
			}
			writeToErr(fderror, "select()");
		} else if(timedout == 0){
		} else {
			if (FD_ISSET(sock, &readfds)){
				if (acceptNbr == 0) { /* Si le nombre de pseudos connus = 1 */
					alarm(30);
				}
				csock[acceptNbr] = acceptSocket(sock, &csin, &sinsize, &buffer, acceptNbr, fderror);
				acceptNbr++;
			} else {
				for (compteur = 0 ; compteur < acceptNbr; compteur++){
					if(FD_ISSET(csock[compteur], &readfds)) {
						n = readSocket(csock[compteur], &buffer, fderror);
						if (n == 0) {//TODO change status -> DISCONNECT etc
							for(i = compteur; i < acceptNbr-1; i++) {
								csock[i] = csock[i+1];
								pseudos[i] = pseudos[i+1];
							}
							acceptNbr--;
							pseudosNbr--;
						} else {
							pseudos[compteur] = buffer.content;
							pseudosNbr++;
						}
					}
				}
			}
		}
	}
	if(sigprocmask(SIG_UNBLOCK, &set, NULL) == -1) {
		writeToErr(fderror, "sigprocmask");
	}
	for(compteur = 0; compteur < acceptNbr; compteur++) {
		buffer.status = 200;
		strcpy(buffer.content, "Lancement de la partie");
		sendSocket(csock[compteur], &buffer, stderr);
	}
	/*  TO DO */
	/*if (i == MAX_PLAYER){
		buffer.status = 500;
		strcpy(buffer.content, "La partie est pleine, vous ne pouvez plus la rejoindre !\n");
		sendSocket(csock[i], &buffer);
	}
	buffer.status = 1;
	memset(buffer.content, 0, sizeof buffer.content);

	sprintf(buffer.content, "La partie commence. Il y a au moins %d joueurs.\n", i);
	for (compteur = 0 ; compteur < i; compteur++)
	{
		sendSocket(csock[compteur], &buffer);
		readSocket(csock[compteur], &buffer);
		printf("Le joueur a répondu : %s\n", buffer.content);

	}*/
	closesocket(sock);
	for(compteur =0; compteur < acceptNbr; compteur++) {
		closesocket(csock[compteur]);
	}
}




/*
 * Receive a message from the specified socket.
 * Returns numbers of caracs readed.
 * Exit in case of error.
 */
int readSocket(SOCKET sock, message *  buffer, FILE * file) {
	int n; /* Number of caracs get by recv */
	if((n = recv(sock, buffer, sizeof((*buffer)) - 1, 0)) < -1) {
		if(errno == EOF) {
			return -1; /* Can be replaced with player.disconnect */
		}
		writeToErr(file, "recv()");
	}
	return n;
	/* Could be improved */
}

/*
 * Send a message to the specified socket.
 * Exit in case of error.
 */
void sendSocket(SOCKET sock, message * buffer, FILE * file) {
	if(send(sock, buffer, sizeof((*(buffer))) -1, 0) < 0) {
		writeToErr(file, "send()");
	}
	/* Could be improved */
}

/*
 * Used to make the server reacting like a singleton.
 * Exit in case of error.
 */
void lock(FILE * file) {
	int f_lock;
	//flock to avoid double server opening
	if ((f_lock = open("serveur.lock", O_RDWR)) == -1){
		writeToErr(file, "open()");
	}
	if (flock(f_lock, LOCK_EX | LOCK_NB) == -1){
		if( errno == EWOULDBLOCK ){

			writeToErr(file, "The server is already launched");
		} else {
			writeToErr(file, "flock");
		}
	}
}

/*
 * Used to initialize the server.
 * Exit in case of error.
 */
void serverInit(int * sock, SOCKADDR_IN * sin, int port, FILE * file) {
	/* Server Initialisation */
	*(sock) = socket(AF_INET, SOCK_STREAM, 0);
	if(*(sock) == INVALID_SOCKET) {
		writeToErr(file, "sock()");
	}
	sin->sin_addr.s_addr = htonl(INADDR_ANY);
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	if(bind(*(sock), (SOCKADDR *) sin, sizeof *(sin)) == SOCKET_ERROR) {
		/*if( errno == EADDRINUSE ) {
			printf("The server is already launched\n");
		} else {*/
		writeToErr(file, "bind()");
		/*}*/
	}
	if(listen(*(sock), MAX_PLAYER) == SOCKET_ERROR) {
		writeToErr(file, "listen()");
	}
}

/*
 * Used to accept a socket.
 * Return the socket created or exit in case of error.
 */
SOCKET acceptSocket(SOCKET sock, SOCKADDR_IN * csin, int * sinsize, message * buffer, int i, FILE * file) {
	SOCKET csock;
	if((csock = accept(sock, (SOCKADDR *) csin, (socklen_t *) sinsize)) == -1) {
		writeToErr(file, "accept()");
	}
	buffer->status = 200;
	sprintf(buffer->content, "Bienvenue sur notre Papayoo's party\nIl y a actuellement %d joueur(s) connectés\n", i + 1);
	sendSocket(csock, buffer, file);
	return csock;
}

/*
 * Used to open a file in the specified mode.
 * Return the FILE * created or exit in case of error.
 */
FILE *openFile(const char * name, const char * mode, FILE * file) {
	FILE *fd;
	if((fd = fopen(name, mode)) == NULL) {
		writeToErr(file, "fopen()");
	}
	return fd;
}

/*
 * Used to write on the error file or stderr.
 */
void writeToErr(FILE * file, char * message) {
	if (file == NULL){
		fprintf(stderr, "%s : %s\n", message, strerror(errno));

	} else {
		fprintf(file, "%s : %s\n", message, strerror(errno));
	}
	exit(errno);
}



