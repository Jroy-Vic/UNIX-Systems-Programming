/* Roy Vicerra - Asgn2 - rll.c */
/* CS 357-01 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rll.h"

/* Initial memory allocation size */
#define BUFFER 100	
#define WORD_BUFFER 50


char *rdline(FILE *file) {
	int c, cnt, lineSize;
	char *line;
	cnt = 0;
	lineSize = BUFFER;
	
	/* Allocate memory */
	line = (char*) malloc(BUFFER);
	if (!line) {				
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	/* Output characters until reaches end of line or file */
	while ((c = getc(file)) != EOF && c != '\n') {		
		if (cnt > lineSize - 2) {					
			lineSize += BUFFER;							
			line = (char*) realloc(line, lineSize);		
			if (!line) {								
				perror("realloc");
				exit(EXIT_FAILURE);
			}
		}

		line[cnt] = c;		
		cnt++; 					
	}
	
	/* Return null pointer if EOF */
	if (c == EOF) {				
		return NULL;
	}

	/* End array with '\0' to make a string */
	line[cnt] = '\0'; 	
	
	/* Free any excess memory, account for '\0' */
	line = (char*) realloc(line, lineSize + 1);			
	if (!line) {										
		perror("realloc");
		exit(EXIT_FAILURE);
	}

	return line; 	
}

char **rdwords(char *line) {
	char *word, **wordptr;
	int i, wordSize, idx, linelen;
	wordptr = (char**) malloc(sizeof(char*) * BUFFER * 100);
	wordSize = 0, idx = 0, linelen = strlen(line);
	if (!wordptr) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}
 	
	/* Create new word */
	if (wordSize == 0) {
	/*		word = (char*) *wordptr[idx]; */
		word = (char*) malloc(WORD_BUFFER * sizeof(char));
		if (!word) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
	}
	
	/* Copy characters into word array until it reaches a non-letter */
	for (i = 0; i < linelen; i++) {
			
	/* If character is alphabetic, add to word as lowercase letter */
		if (isalpha(line[i])) {
			word[wordSize] = tolower((int)line[i]);
			wordSize++;
		
		/* If not alphabetic and at the end of word */
		} else if (wordSize > 0) {
			word[i] = '\0';
		
			/* Free excess memory */
			word = (char*) realloc(word, (wordSize + 1) * sizeof(char)); 	
			if (!word) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
			wordptr[idx] = (char*) malloc(sizeof(word));
			if (!wordptr[idx]) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}

			/* Store word and reinitialize*/
		 	strcpy(wordptr[idx], word);
			wordSize = 0;
			idx++;
			
			free(word);
			word = (char*) malloc(WORD_BUFFER * sizeof(char)); 	
			if (!word) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
		}
	}

	/* Check in case line ended with alphabetic letter */
	if (wordSize > 0) {
		word[wordSize] = '\0';
		wordptr[idx] = (char*) malloc(sizeof(word));	
		if (!wordptr[idx]) {
			perror("nalloc");
			exit(EXIT_FAILURE);
		}
		strcpy(wordptr[idx], word);
		idx++;
	}

	wordptr[idx] = NULL;
	free(word);
	
	return wordptr;
}	
