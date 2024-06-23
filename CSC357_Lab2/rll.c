/* Roy Vicerra - Lab02 - rll.c
 * CS 357-01
*/

#include <stdio.h>
#include <stdlib.h>
#include "rll.h"

#define BUFFER 100				/* Initial size of memory allocation */

char *read_long_line(FILE *file) {
	int c, cnt = 0, lineSize = BUFFER;
	char *line = (char*) malloc(BUFFER);
	if (!line) {				/* Check if memory is allocated */
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	while ((c = getc(file)) != EOF && c != '\n') {		/* End string line if EOF or \n */	
		if (cnt > lineSize - 2) {						/* Account for last char and \0 */
			lineSize += BUFFER;							
			line = (char*) realloc(line, lineSize);		/* Reallocate memory if need be */
			if (!line) {								/* Check if memory is reallocated */
				perror("realloc");
				exit(EXIT_FAILURE);
			}
		}

		*(line + cnt) = c;		/* Place char into address */
		cnt++; 					/* Advance to next address */
	}
	
	if (c == EOF) {				/* End program if EOF */
		return NULL;
	}

	*(line + cnt) = '\0'; 		/* End string with \0 */
	
	line = (char*) realloc(line, lineSize + 1);			/* Free excess, unused memory */
	if (!line) {										/* Check if memory is reallocated */
		perror("realloc");
		exit(EXIT_FAILURE);
	}

	return line; 	
}	
