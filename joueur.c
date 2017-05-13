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
	char * charBuf;
	char * charBuf2;
	const char *hostname;
	card * cards;
	int n = 0, o = 0, port, cardsNumber;
	SOCKADDR_IN sin = { 0 };
	struct sigaction interrupt;
	sigset_t set;
	if(argc != 3) {
		fprintf(stderr, "joueur <port> <ipHost>\n");
		return ERROR;
	}
	SYSN((charBuf2) = (char *) malloc(sizeof(char) * 256));
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
	keyboardReader(&charBuf);
	strcpy(buffer.content, charBuf);
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
		if(buffer.status == 202) {
			printf("card : %s\n", buffer.content);
		}
	}

	/* Game's Beginning - End of Login Phase */
	mReader(&sem, &nbLect, &shm, PLAYERS);

	/* Card's first draw */	
	n = 0;
	SYSN(cards = (struct card*) malloc(sizeof(struct card)*30));	
	while(1){
		readJ(&buffer);
		fflush(stdin);
		if(buffer.status == 202) {
			cards[n] = createCard(atoi(buffer.content));
			printf("Carte %d - %d de %s.\n",n+1,cards[n].value,cards[n].color);
			n++;
		}
		if(buffer.status == 500) {
			break;
		}
	}
	cardsNumber = n;
	printf("Veuillez entrer les 5 numéros de carte à écarter.\n");
	fflush(stdin);
	n=0, o = 0;
	strcpy(buffer.content, "");
	while (n<5){
		keyboardReader(&charBuf);
		o = atoi(charBuf)-1;
		/* check if the card isn't already picked */
		char* tempstr = calloc(strlen(buffer.content)+1, sizeof(char));
		strcpy(tempstr, buffer.content);
		charBuf2 = strtok(tempstr, " ");
  		while (charBuf2 != NULL) {
			if (atoi(charBuf2) == o+1) {
			o = -1;
			}
    		 	charBuf2 = strtok(NULL, " ");
  		}
		if (o < 0 || o > cardsNumber){
			printf("Vous devez entrer un nombre valide. Non déjà entré et compris entre 1 et %d !\n",cardsNumber);
		} else {

		charBuf[strcspn(charBuf, "\n")] = 0;
		cards[o].value = -1;
		strcpy(cards[o].color,"");
		cards[o].id = -1;
		strcat(buffer.content, strcat(charBuf," "));
		n++;
		}
	}
	printf("You removed cards %s\n You are now waiting for every player to remove their cards.\n",buffer.content);
	sendJ(&buffer);
	strcpy(charBuf, "");
	n=0;
	while(1){
		readJ(&buffer);	
		if (buffer.status == 203) {	
			printf("You got these cards : %s\n",buffer.content);
  			charBuf = strtok(buffer.content, " ");
  			while (charBuf != NULL) {
				while (cards[n].id != -1){
					n++;
					if (n>30) {
						printf("Erreur lors de la réception des cartes écartées.");
						break;
					}
				}
				cards[n] = createCard(atoi(charBuf));
				n=0;
    			 	charBuf = strtok(NULL, " ");
  			}
			break;
		}
	}
	printf("Marvelous, here's your hand !\n");
	n=0;
	while (cardsNumber > n) {
		printf("Carte %d - %d de %s.\n",n+1,cards[n].value,cards[n].color);
		n++;	
	}
	printf("You are waiting for your turn. You can check things if you do things [TODO]\n");


	printf("End of the game\n");		
	close(sock);
	closeIPCs(&shm, &nbLect, &sem);
}


/*
 * Used to create a card from it's id.
 */
card createCard(int id){
	card toReturn;
	toReturn.id = id;
	switch(id){
		case 0 ... 9 :
			strcpy(toReturn.color,"Coeur");
			toReturn.value = id-0;
		break;
		case 10 ... 19:
			strcpy(toReturn.color,"Carreau");
			toReturn.value = id-10;
		break;
		case 20 ... 29:
			strcpy(toReturn.color,"Trèfle");
			toReturn.value = id-20;
		break;
		case 30 ... 39:
			strcpy(toReturn.color,"Pique");
			toReturn.value = id-30;
		break;
		case 40 ... 59:
			strcpy(toReturn.color,"Papayoo");
			toReturn.value = id-40;
		break;
	}
	return toReturn;
}

/*
 * Used to readJ an input from the keyboard.
 */
void keyboardReader(char** charBuf){
	SYSN((*charBuf) = (char *) malloc(sizeof(char) * 256));
	SYSN((fgets(*(charBuf), NAME_LENGTH, stdin)));
	if((*charBuf)[strlen(*charBuf)-1] != '\n'){
		perror("Trop grande ligne lue\n");
		while((*charBuf)[strlen(*charBuf) -1] != '\n'){
			fgets(*(charBuf), NAME_LENGTH, stdin);
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
