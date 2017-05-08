/*
 * ==================================================================
 *
 * Filename : serveur.h
 *
 * Description : Header of the server.c file
 *
 * Author : MANIET Antoine "nomLogin", SACRE Christopher "csacre15"
 *
 * ==================================================================
 */

#ifndef SERVEURH_H
#define SERVEURH_H

#define SYS(call) ((call) == -1) ? exit(1) : 0

typedef struct player {
  int pseudoKnown;
  char * pseudo;
  SOCKET sock;
}player;


#endif

void lock(FILE * file);
FILE *openFile(const char * name, const char * mode, FILE * file);
void serverSigaction(struct sigaction *act, struct sigaction *actInt, sigset_t *set, FILE * fderror);
