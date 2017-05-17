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
	closeIPCs(&shm, &nbLect);
	close(sock);
	exit(0);
}
int main(int argc, char** argv) {
	/* Vérification des arguments */
	if(argc != 3) {
		fprintf(stderr, "joueur <port> <ipHost>\n");
		return ERROR;
	}

	/* Définition des variables */
	message buffer;
	char ligne[1024];
	char * charBuf;
	char * charBuf2;
	char* tempstr;
	const char *hostname;
	card * cards;
	card tmpCard;
	char colorOfTheTurn[8];
	int numberLeft = 0;
	int timedout = 0, n = 0, o = 0, port, cardsNumber, co = 0, ca = 0, tr = 0, pi = 0, pa = 0, notYourTurn = 1;
	SOCKADDR_IN sin = { 0 };
	struct sigaction interrupt;
	struct timeval tv;
	fd_set readfds;
	sigset_t set;
	SYSN(cards = (struct card*) malloc(sizeof(struct card)*30));
	SYSN((charBuf2) = (char *) malloc(sizeof(char) * 256));
	port = atoi(*++argv);
	hostname = *++argv;
	sock = joueurInit(hostname, &sin, port);
	setHandler(&interrupt, &set);
	initSharedMemory(&shm, &nbLect, &sem);

	/* Trying to connect to the server */
	if(connect(sock, (SOCKADDR *) &sin, sizeof(SOCKADDR)) == -1) {
		if(errno == ECONNREFUSED) {
			printf("Le serveur est actuellement hors ligne.\n");
		} else {
			perror("connect()");
		}
		exit(errno);
	}

	/* Showing Welcome message */
	/* Waiting for the server */
	readJ(&buffer);
	printf("%s", buffer.content);
	if(buffer.status == 500) {
		exit(0);
	}

	/* readJ-ing the user's name */
	printf("Veuillez entrer votre pseudo : \n");
	fflush(stdin);
	keyboardReader(&charBuf);
	strcpy(buffer.content, charBuf);
	buffer.status = 200;
	sendJ(&buffer);
	if (buffer.status == 500){
		exit(0);
	}

	printf("Vous êtes actuellement en attente d'une réponse du serveur...\n");
	while(1) {
		/* Waiting for the server */
		readJ(&buffer);
		printf("%s\n", buffer.content);
		fflush(stdin);
		if(buffer.status == 201) {
			break;
		}
	}

	/* Game's Beginning - End of Login Phase */

	/* Shows the players */
	mReader(&sem, &nbLect, &shm, PLAYERS);
	while(1){
		/* Print payoo */
		/* Waiting for the server */
		readJ(&buffer);
		/* Fin partie */
		if (buffer.status == 210){
			printf("%s\n", buffer.content);
			fflush(stdin);
			break;
		}
		printf("Le payoo est %s\n\nVoici vos cartes :\n", buffer.content);
		fflush(stdin);
		/* Card's draw */
		n = 0;
		while(1){
			/* Waiting for the server */
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
			if (cards[o].id == -1) {
				printf("Déjà sélectionnée\n");
				o = -1;
			}
			if (o < 0 || o > cardsNumber){
				printf("Vous devez entrer un nombre valide. Non déjà sélectionné et compris entre 1 et %d !\n",cardsNumber);
			} else {
				charBuf[strcspn(charBuf, "\n")] = 0;
				tempstr = calloc(strlen(buffer.content)+1, sizeof(char));
				strcpy(tempstr,"");
				sprintf(tempstr, "_%d_%d",cards[o].id, cards[o].value);
				strcat(buffer.content, strcat(strcat(strcat(tempstr," de "),cards[o].color),", "));
				cards[o].value = -1;
				strcpy(cards[o].color,"");
				cards[o].id = -1;
				n++;
			}
		}
		printf("Vous avez écarté des cartes et attendez désormais que chaque joueur ait fait de même.\n");
		sendJ(&buffer);
		strcpy(charBuf, "");
		n=0;
		while(1){
			/* Waiting for the server */
			readJ(&buffer);
			if (buffer.status == 203) {
				printf("Vous recevez :\n");
				charBuf = strtok(buffer.content, "_");
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
					charBuf = strtok(NULL, "_");
					printf("%s\n",charBuf);
					charBuf = strtok(NULL, "_");
				}
				break;
			}
		}
		printf("Voici votre main :\n");
		n=0;
		while (cardsNumber > n) {
			printf("Carte %d - %d de %s.\n",n+1,cards[n].value,cards[n].color);
			if(!strcmp(cards[n].color, COEUR)){
				co++;
			} else if(!strcmp(cards[n].color, CARREAU)){
				ca++;
			} else if(!strcmp(cards[n].color, TREFLE)){
				tr++;
			} else if(!strcmp(cards[n].color, PIQUE)){
				pi++;
			} else if(!strcmp(cards[n].color, "Papayoo")){
				pa++;
			}
			n++;
		}

		while(1){
			printf("Vous attendez votre tour.\n");
			fflush(stdin);
			while(1) {
				printf("Vous pouvez consulter des informations en attendant.\n1. Afficher les scores\n2. Afficher mes cartes\n");
				tv.tv_sec = 60;
				tv.tv_usec = 0;
				FD_ZERO(&readfds);
				FD_SET(sock, &readfds);
				FD_SET(0, &readfds);
				if((timedout = select(sock + 1, &readfds, NULL, NULL, &tv)) == ERROR) {
					perror("select()");
					exit(ERRNO);
				} else if(timedout != 0) {
					if(FD_ISSET(sock, &readfds)) {
						printf("Attention votre tour commence\n");
						break;
					} else if(FD_ISSET(0, &readfds)) {
						keyboardReader(&charBuf);
						if(atoi(charBuf) == 1) {
							mReader(&sem, &nbLect, &shm, SCORE);
						} else if(atoi(charBuf) == 2) {
							n=0;
							while (cardsNumber > n) {
								if(cards[n].id ==-1) {
									n++;
									continue;
								}
								printf("Carte %d - %d de %s.\n",n+1,cards[n].value,cards[n].color);
								n++;
							}
						} else {
							printf("Mauvais chiffre entré, veuillez réitérer votre opération\n");
						}
					}
				}
			}
			/* Waiting for the server */
			readJ(&buffer);
			while (buffer.status == 204) {
				strcpy(colorOfTheTurn,"Empty");
				numberLeft = 0;
				tempstr = calloc(strlen(buffer.content)+1, sizeof(char));
				strcpy(tempstr, buffer.content);
				charBuf = strtok(tempstr, "_");
				/* Message about cards already played this turn */
				printf("%s\n",charBuf);
				charBuf = strtok(NULL, "_");
				while (charBuf != NULL) {
					tmpCard = createCard(atoi(charBuf));
					printf("%d de %s.\n",tmpCard.value,tmpCard.color);
					if (!strcmp(colorOfTheTurn, "Empty")){
						if(!strcmp(tmpCard.color, COEUR)){
							strcpy(colorOfTheTurn, tmpCard.color);
							numberLeft = co;
						}
						else if(!strcmp(tmpCard.color, CARREAU)){
							strcpy(colorOfTheTurn, tmpCard.color);
							numberLeft = ca;
						}
						else if(!strcmp(tmpCard.color, TREFLE)){
							strcpy(colorOfTheTurn, tmpCard.color);
							numberLeft = tr;
						}
						else if(!strcmp(tmpCard.color, PIQUE)){
							strcpy(colorOfTheTurn, tmpCard.color);
							numberLeft = pi;
						}
						else if(!strcmp(tmpCard.color, "Papayoo")){
							strcpy(colorOfTheTurn, tmpCard.color);
							numberLeft = pa;
						}
					}
					charBuf = strtok(NULL, "_");
				}
				fflush(stdin);
				n=0, o=-1;
				strcpy(buffer.content, "");
				while (o == -1){
					printf("Veuillez entrer le numéro de la carte à jouer.\n");
					keyboardReader(&charBuf);
					o = atoi(charBuf)-1;
					/* check if the card isn't already played */
					if (shm->cards[o].id > 0 || cards[o].id == -1) {
						o = -1;
					}
					/* check if the cards color is ok */
					if (numberLeft > 0 && strcmp(cards[o].color, colorOfTheTurn)){
						o = -1;
					}
					if (o < 0 || o > cardsNumber){
						printf("Vous devez entrer un id de carte valide.\n(Encore en main et de la couleur du tour s'il vous en reste. ID toujours entre 1 et %d !)\n",cardsNumber);
						o = -1;
					} else {
						buffer.status = 500;
						sprintf(buffer.content,"%d", cards[o].id);
						cards[o].value = -1;
						strcpy(cards[o].color,"");
						cards[o].id = -1;
						sendJ(&buffer);
					}
				}
			}
			/* Fin tour */
			if (buffer.status == 205){
				printf("%s\n", buffer.content);
			}
			/* Fin manche */

			if (buffer.status == 206){
				/* Waiting for the server */
				/*readJ(&buffer);*/
				printf("%s\n", buffer.content);
				printf("_____________________________________________________________\n");
				fflush(stdin);
				break;
			}
		}
	}
	readJ(&buffer);
	/*printf("%s", buffer.content);*/
	close(sock);
	closeIPCs(&shm, &nbLect);
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
 * Used to readJ an input from the keyboard.
 */
void keyboardReader(char** charBuf){
	SYSN((*charBuf) = (char *) malloc(sizeof(char) * 256));
	if ((fgets(*(charBuf), NAME_LENGTH, stdin)) == NULL) {
		printf("Nous fermons votre connexion ....\n");
		closesocket(sock);
		closeIPCs(&shm, &nbLect);
		exit(ERRNO);
	} else if((*charBuf)[strlen(*charBuf)-1] != '\n'){
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
		closeAllIPCs(&shm, &nbLect, &sem); /* If time change to close all IPCs */
		closesocket(sock);
		exit(errno);
	}
	if(buffer->status == 600) {
		printf("%s", buffer->content);
		closeIPCs(&shm, &nbLect);
		closesocket(sock);
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
