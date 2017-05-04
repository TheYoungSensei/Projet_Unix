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
#define SYS(call) ((call) == -1) ? perror(#call " : ERROR "), exit(1) : 0

void timeout(int bla){
	return;
}
int main(int argc, char** argv) {
	SOCKET sock;
	SOCKET csock[MAX_PLAYER];
	char * pseudos[MAX_PLAYER];
	message buffer;
	struct timeval tv;
	struct sigaction act;
	sigset_t set;
	act.sa_handler = timeout;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	SYS(sigfillset(&set));
	SYS(sigdelset(&set, SIGALRM));
	fd_set readfds;
	int fd_stdin = fileno(stdin);
	pid_t cpid;
	FD_ZERO(&readfds);
	FD_SET(fileno(stdin), &readfds); //Ajoute les ensemble stdin etc dans readfds. Cela permet au select de lire les fd donnés.
	tv.tv_sec = 50;//ou récupérer via argv
	tv.tv_usec = 0;
	int timedout;
	int maxFd = sock;
	int port, n = 0, i = 0;
	const char *hostname = "127.0.0.1";
	SOCKADDR_IN sin;
	SOCKADDR_IN csin;


	//flock to avoid double server opening
	int f_lock;
	if ((f_lock = open("serveur.lock", O_RDWR)) == -1){
		perror("open()\n");
		exit(errno);
	}
	if (flock(f_lock, LOCK_EX | LOCK_NB) == -1){
		if( errno == EWOULDBLOCK ){
			printf("The server is already launched\n");
		} else {
			printf("flock()\n");
		}
		exit(errno);
	}
	if(argc != 3) {
		fprintf(stderr, "serveur [numPort] [stderr]\n");
		return EXIT_ERROR;
	}
	port = atoi(*++argv);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == INVALID_SOCKET) {
		perror("socket()");
		exit(errno);
	}
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	if(bind(sock, (SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR) {
		if( errno == EADDRINUSE )
		{
			printf("The server is already launched\n");
		} else {
			perror("bind()");
			exit(errno);
		}
	}
	if(listen(sock, MAX_PLAYER) == SOCKET_ERROR) {
		perror("listen()");
		exit(errno);
	}
	int sinsize = sizeof csin;

	SYS(sigprocmask(SIG_BLOCK, &set, NULL));
	sigaction(SIGALRM, &act, NULL);
	while(1)
	{
		if (i==0){
			csock[i] = accept(sock, (SOCKADDR *) &csin, (socklen_t *) &sinsize);
			buffer.status = 200;
			sprintf(buffer.content, "Bienvenue sur notre Papayoo's party\n Il y a actuellement %d joueur(s) connectés\n", i + 1);
			if(send(csock[i], &buffer, sizeof(buffer) -1, 0) < 0) {
				perror("send()");
				exit(errno);
			}
			i++;
			continue;
		}
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		for (int compteur = 0 ; compteur < i; compteur++){
			FD_SET(csock[compteur], &readfds);
			if (csock[compteur]>maxFd) maxFd = csock[compteur];
		}
		SYS(timedout = select(maxFd+1, &readfds, NULL, NULL, &tv));
		if(timedout == -1){
			if (errno == EINTR){
				printf("TEST");
				//close socket
			}
			break;
		}
		else if(timedout == 0){
			//TODO Début de partie
			break;
		} else {
			if (FD_ISSET(sock, &readfds)){
				csock[i] = accept(sock, (SOCKADDR *) &csin, (socklen_t *) &sinsize);
				buffer.status = 200;
				sprintf(buffer.content, "Bienvenue sur notre Papayoo's party\n Il y a actuellement %d joueur(s) connectés\n", i + 1);
				if(send(csock[i], &buffer, sizeof(buffer) -1, 0) < 0) {
					perror("send()");
					exit(errno);
				}
				i++;
			} else {
				for (int compteur = 0 ; compteur < i; compteur++){
					if(FD_ISSET(csock[compteur], &readfds)) {
						if((n = recv(csock[compteur], &buffer, sizeof(buffer) - 1, 0)) < 0) {
							perror("recvName()");
							exit(errno);
						}
						if (buffer.status == 401){//TODO change status -> DISCONNECT etc
							csock[compteur] = csock[i];
							//clean socket
							i--;
						} else {
							if (1)// si la taille de pseudos est à 0
							{
								alarm(30);
							}
							printf("%s", buffer.content);
							pseudos[compteur] = buffer.content;
						}
					}
				}
			}
		}
	}
	SYS(sigprocmask(SIG_UNBLOCK, &set, NULL));


	if (i == MAX_PLAYER){
		buffer.status = 500;
		sprintf(buffer.content, "La partie est pleine, vous ne pouvez plus la rejoindre !\n");
		if(send(csock[i], &buffer, sizeof(buffer) -1, 0) < 0) {
			perror("send()");
			exit(errno);
		}
	}
	buffer.status = 1;
	memset(buffer.content, 0, sizeof buffer.content);

	sprintf(buffer.content, "La partie commence. Il y a au moins %d joueurs.\n", i);
	for (int compteur = 0 ; compteur < i; compteur++)
	{
		if(send(csock[compteur], &buffer, sizeof(buffer) -1, 0) < 0) {
			perror("send()");
			exit(errno);
		}
		/* RECEPTION */
		if((n = recv(csock[compteur], &buffer, sizeof(buffer) - 1, 0)) < 0) {
			perror("recv()");
			exit(errno);
		}
		printf("Le joueur a répondu : %s\n", buffer.content);

	}
	if(csock[i] == INVALID_SOCKET) {
		perror("accept()");
		exit(errno);
	}
	closesocket(sock);
	for(i =0; i < MAX_PLAYER; i++) {
		closesocket(csock[i]);
	}
}







