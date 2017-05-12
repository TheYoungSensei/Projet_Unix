/*
 * ==================================================================
 *
 * Filename : joueur.c
 *
 * Description : file used to play the client role.
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */

#include "joueur.h"

/* Needed outside of the main and more precisely in interruptHandler */
memory *shm;
int * nbLect;
semaphore *sem;
SOCKET sock;

void interruptHandler(int sigint) {
	printf("Signal %d reçu\n", sigint);
	closeIPCs(&shm, &nbLect, &sem);
	close(sock);
}

int main(int argc, char** argv) {
	message buffer;
	char ligne[1024];
	char * name;
	const char *hostname;
	int n = 0, port;
	SOCKADDR_IN sin = { 0 };
	struct sigaction interrupt;
	sigset_t set;
	if(argc != 3) {
		fprintf(stderr, "joueur <port> <ipHost>\n");
		return ERROR;
	}
	port = atoi(*++argv);
	hostname = *++argv;
	sock = joueurInit(hostname, &sin, port);
	setHandler(&interrupt, &set);
	initSharedMemory(&shm, &nbLect, &sem);
	/* Trying to connect to the server */
	SYS(connect(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)));
	/* Showing Welcome message */
	readJ(&buffer);
	printf("%s", buffer.content);
	/* readJing the userName */
	printf("Veuillez entrer votre pseudo : \n");
	fflush(stdin);
	keyboardReader(&name);
	strcpy(buffer.content, name);
	buffer.status = 200;
	sendJ(&buffer);
	if (buffer.status == 500){
		/* TO DISCUSS */
		exit(0);
	}
	printf("Vous êtes actuellement en attente d'une réponse du serveur...\n");
	/* Future interactions with the serveur */
	while(1) {
		readJ(&buffer);
		printf("%s\n", buffer.content);
		fflush(stdin);
		if(buffer.status == 201) {
			break;
		}
	}
	/* Game's Beginning - End of Login Phase */
	mReader(&sem, &nbLect, &shm, PLAYERS);
	close(sock);
	closeIPCs(&shm, &nbLect, &sem);
}

/*
 * Used to readJ an input from the keyboard.
 */
void keyboardReader(char** name){
	SYSN(((*name) = (char *) malloc(sizeof(char) * NAME_LENGTH)));
	SYSN((fgets(*(name), NAME_LENGTH, stdin)));
	if((*name)[strlen(*name)-1] != '\n'){
		perror("Trop grande ligne lue\n");
		while((*name)[strlen(*name) -1] != '\n'){
			fgets(*(name), NAME_LENGTH, stdin);
			exit(11);
		}
	}
	fflush(stdin);
}

/*
 * Receive a message from the specified socket.
 * Returns number of caracs readJ in case of success.
 * Exit in case of error.
 */
int readJ(message *  buffer) {
	int n; /* Number of caracs get by recv */
	n = readSocket(sock, buffer);
	if (n == 0) {
		printf("Le serveur s'est malheureusement déconnecté\n");
		exit(errno);
	}
	return n;
}

/*
 * sendJ a message to the specified socket.
 * Exit in case of error.
 */
void sendJ(message * buffer) {
	sendSocket(sock, buffer);
}

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
