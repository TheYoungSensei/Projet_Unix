
/*
 * ==================================================================
 *
 * Filename : global.c
 *
 * Description : global file.
 *
 * Author : MANIET Antoine "amaniet152" (Série : 2), SACRE Christopher "csacre15" (Série : 2)
 *
 * ==================================================================
 */

 #include "global.h"

/*
 * Used to write on the error file or stderr.
 */
void writeToErr(FILE * file, char * message) {
	if (file == NULL){
		fprintf(stderr, "%s : %s\n", message, strerror(errno));

	} else {
		fprintf(file, "%s : %s\n", message, strerror(errno));
	}
}

void initSharedMemory(char ** shm, char ** nbLect, semaphore * sem) {
int shmid;
shmid = getMemory();
*shm = attachMemory(shmid);
shmid = getMemory();
*nbLect = attachMemory(shmid);
*sem = sembufInit();
}
