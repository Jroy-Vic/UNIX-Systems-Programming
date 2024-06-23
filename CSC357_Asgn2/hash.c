/* Roy Vicerra - Asgn2 - hash.c */
/* CSC 357-01 */

#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define HASH 5381

int hashfunc(hashtb *ht, char *word) {

	/* Produce hashcode for each word */
	int hashidx = 0, wordlen, i;
	wordlen = strlen(word);
	for (i = 0; i < wordlen; i++) {
		hashidx = (((HASH << 5) + HASH) + word[i]) % ht->size;
	}
	return hashidx;
}


hashtb *hashtb_create(int size) {
	
	/* Initialize and allocate memmory for hash table */
	hashtb *ht = (hashtb*) malloc(sizeof(hashtb));
	if (!ht) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	ht->size = size;
	ht->wordnum = 0;

	/* Elements in hashtable are calloced to represent empty buckets */
	ht->element = (entry**) calloc(sizeof(entry), ht->size);
	if (!ht->element) {
		perror("calloc");
		exit(EXIT_FAILURE);
	}
	return ht;
}


void hashtb_print(hashtb *ht) {

	/* Easy to debug hashtable using printf */
	int i;
	entry *elementptr;
	printf("Start of Hash Table:\n");
	for (i = 0; i < ht->size; i++) {
		if (ht->element[i] == 0) {
			printf("\t%i\t--(Empty)--",i);
		} else {
			printf("\t%i",i);
			elementptr = ht->element[i];
			printf("\t%s, Count: %d\t",elementptr->word,elementptr->count);
			elementptr = elementptr->next;
			while (elementptr != 0) {
				printf("   --->   %s, Count: %d\t",elementptr->word, elementptr->count);
				elementptr = elementptr->next; 
			}
		}
		printf("\n\n");
	}
	printf("End of Hash Table\n");
}


void hashtb_insert(hashtb **htptr, char *word) {
	float loadfactor;
	entry *elin, *elnext;		
	
	/* Check if word is valid */						
	if (word != NULL) {			
		int idx = hashfunc(*htptr, word);
		elin = (*htptr)->element[idx];

	/* Insert new word, or add count */
		if (elin == 0) {						
			elin = (entry*) malloc(sizeof(entry));
			elin->word = (char*) malloc((strlen(word) * sizeof(char)) + 1);
			if (!(elin->word)) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}
			strcpy(elin->word, word);
			elin->count = 1;
			elin->next = 0;
			(*htptr)->element[idx] = elin;
			(*htptr)->wordnum += 1;
		} else if (strcmp(word, elin->word) == 0) {	
			elin->count += 1;

	/* If same hashcode, add onto linked list or add count */
		} else { 
			elnext = elin->next;
			if (elin->next == 0) {
				elnext = (entry*) malloc(sizeof(entry));
				elnext->word = (char*) malloc((strlen(word) * sizeof(char)) + 1);
				if (!(elnext->word)) {
					perror("malloc");
					exit(EXIT_FAILURE);
				}
				strcpy(elnext->word, word);
				elnext->count = 1;
				elnext->next = 0;
				elin->next = elnext;
				(*htptr)->wordnum += 1;		 
			} else if (strcmp(word, elnext->word) == 0) {
				elnext->count += 1;
			}  
		}

	/* Check if loadfactor exceeds threshold; rehash if so */
		loadfactor = (float) (*htptr)->wordnum / (*htptr)->size;
		if (loadfactor >= 0.75) {
			*htptr = hashtb_rehash(*htptr);
		}		
	}
}


void hashtb_hashcpy(hashtb *ht, hashtb *newht, char *word) {
	entry *elin, *elnext;		
	int oldidx = hashfunc(ht, word);
	
	/* Rehash word into new hashtable */
	int idx = hashfunc(newht, word);
	elin = newht->element[idx];

	/* If empty bucket, copy word */
	if (elin == 0) {						
		elin = (entry*) malloc(sizeof(entry));
		elin->word = (char*) malloc((strlen(word) * sizeof(char)) + 1);
		if (!(elin->word)) {
			perror("malloc");
			exit(EXIT_FAILURE);	
		}
		strcpy(elin->word, word);
		elin->count = (ht->element[oldidx])->count;
		elin->next = 0;
		newht->element[idx] = elin;

	/* If same hashcode, copy onto linked list */
	} else { 
		elnext = elin->next;
		if (elin->next == 0) {
			elnext = (entry*) malloc(sizeof(entry));
			elnext->word = (char*) malloc((strlen(word) * sizeof(char)) + 1);
			if (!(elnext->word)) {
				perror("malloc");
				exit(EXIT_FAILURE);
			}
			strcpy(elnext->word, word);
			elnext->count = (ht->element[oldidx])->count;
			elnext->next = 0;
			elin->next = elnext;
		} 
	}

}



hashtb *hashtb_rehash(hashtb *ht) {
	int i, htlen = ht->size;
	entry *tmp;
	
	/* Create new hashtable with double the size */
	hashtb *newht = hashtb_create(((ht->size) * 2));
	newht->wordnum = ht->wordnum;

	/* Rehash all existing contents into new hashtable */
	tmp = (entry*) malloc(sizeof(entry));
	for (i = 0; i < htlen; i++) {
		if ((tmp = ht->element[i]) != 0) {
			hashtb_hashcpy(ht, newht, tmp->word);
			while (tmp->next != 0) {
				hashtb_hashcpy(ht, newht, (tmp->next)->word);
				tmp = tmp->next;
			}
		
	/* Free memory from old hashtable */ 
			free(tmp->word);
		} 
	}
	free(tmp);
	free(ht);

	return newht;  	
}

