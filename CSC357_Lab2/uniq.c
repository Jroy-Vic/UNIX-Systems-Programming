/* Roy Vicerra - Lab02 - uniq.c
 * CS 357-01
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rll.h"

int main() {
	char *fileLine, *nextLine;
	fileLine = read_long_line(stdin);			/* Get first line */
	while (fileLine && (nextLine = read_long_line(stdin))) {	/* Reads and verifies next line */
		if (strcmp(fileLine, nextLine)) {		/* strcmp() = 0 if duplicate */
	    	puts(fileLine);					
			free(fileLine);

			int nextLen = strlen(nextLine);		/* Used to determine memory allocation to copy line */
			fileLine = (char*) malloc((nextLen + 1) * sizeof(char));	/* Account for \0 */
			if (!fileLine) {					/* Check if memory is allocated */
				perror("malloc");
				exit(EXIT_FAILURE);
			}

			strcpy(fileLine, nextLine);			/* Copy second string to first string */
			free(nextLine);						/* Free second line to read next line in file */
		}
		else {
			free(nextLine);					/* Move onto next line if duplicate */
		}
	}

	if (fileLine) {							/* Prints last line */	
		puts(fileLine);
	}
	
	free(fileLine);							/* Deallocate memory when finished */
	free(nextLine);
	
	return 0;
}
