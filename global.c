
/*
 * ==================================================================
 *
 * Filename : global.c
 *
 * Description : global file.
 *
 * Author : MANIET Antoine "nomLogin", SACRE Christopher "csacre15"
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
