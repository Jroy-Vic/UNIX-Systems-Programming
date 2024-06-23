/* Roy Vicerra - Asgn2 - hash.h */
/* CSC 357-01 */

#ifndef HASH_H
#define HASH_H

/* Element in each bucket of hashtable */
typedef struct entry {
	char *word;
	int count;
	struct entry *next;
} entry;

/* Hashtable */
typedef struct HashTable {
	int size;
	int wordnum;
	entry **element;
} hashtb;

/* Generate hashcode based on inputted word */
int hashfunc(hashtb *ht, char *word);

/* Create hashtable based on inputted size*/
hashtb *hashtb_create(int size);

/* Print hashtable values */
void hashtb_print(hashtb *ht);

/* Add element to hashtable */
void hashtb_insert(hashtb **ht, char *word);

/* Rehashing: copies old hashtable values to new hashtable */
void hashtb_hashcpy(hashtb *ht, hashtb *newht, char *word);

/* Increase the size of hashtable */
hashtb *hashtb_rehash(hashtb *ht);

#endif 
