/* Roy Vicerra -  Asgn2 - fw.c */
/* CSC 357-01 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "hash.h"
#include "rll.h"

#define FILE_BUFFER 10
#define HASH_SIZE 50


int main(int argc, char *argv[]) {
	char *fileline, **words, *endptr;
	int i, idx = 0, fileidx = 0, check, argcheck = 0;
	long *argptr = NULL, arginput = 0, numwords;
	char **fileinput; 
	FILE *currfile;
	hashtb **testtbptr, *testtb; 

	/* Check for command line arguments */	
	fileinput = (char**) malloc(sizeof(char*) * FILE_BUFFER);
	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-n") == 0) {
				argptr = &arginput; 
				argcheck++;
			} else if (isdigit(argv[i][0])) {
				arginput = strtol(argv[i], &endptr, 10);
				argcheck++;
				if (*endptr != '\0') {
					fprintf(stderr, "Invalid number: %s\n", argv[i]);
					exit(EXIT_FAILURE);
				} 
			} else {
				fileinput[fileidx] = (char*) malloc(strlen(argv[i]) + 1);
				strcpy(fileinput[fileidx], argv[i]);
				if (!fileinput[fileidx]) {
					perror("Error opening file");
					exit(EXIT_FAILURE);
				}
				fileidx++;
				check = 1; 
			}
		}

		if (argcheck > 0 && argcheck != 2) {
			perror("invalid argument");
			printf("Invalid argument.\n");
			exit(EXIT_FAILURE);
		} else if (argcheck == 2) {
			numwords = *argptr;
		}
		
		free(argptr);
		if (!fileinput) {
			free(fileinput);
		}
		fileidx = 0;
	
	/* If no arguments, set to default settings */	
	} else {
		numwords = 10;
		printf("Enter a line of words: \n");
		fileline = rdline(stdin);
		check = 0;
	}

	/* Create Hashtable */
	testtb = hashtb_create(HASH_SIZE);
	testtbptr = &testtb;
	hashtb_print(testtb);

	/* If there are inputted files, read data from them */
	while (fileinput[fileidx] && check == 1) {
		currfile = fopen(fileinput[fileidx], "r");
		if (currfile == NULL) {
			perror("No such file or directory");
			exit(EXIT_FAILURE);
		}
		
		while((fileline = rdline(currfile))) {
			printf("%s", fileline);
			words = rdwords(fileline);
			idx = 0;
			while (words[idx]) {
				printf("Word[%d]: %s\n", idx, words[idx]);
				hashtb_insert(testtbptr, words[idx]);
				free(words[idx]);
				idx++;
			}
			free(words);
			free(fileline);		
		}
		fclose(currfile);
		free(fileinput[fileidx]);
		fileidx++;
	}
	free(fileinput);

	/* If no file(s) inputted, continue using stdin */
	if (check == 0) {
		words = rdwords(fileline);

		while (words[idx]) {
			printf("Word[%d]: %s\n", idx, words[idx]);
			idx++;
		}	

		idx = 0;
		while (words[idx]) {	
			hashtb_insert(testtbptr, words[idx]);
			idx++;
		}
	} 

	hashtb_print(testtb);
	
	
	return 0;
}
