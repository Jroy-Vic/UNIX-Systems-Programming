#ifndef LINK_H
#define LINK_H

#include <stdlib.h>

typedef struct Node {
    char character;
    int count;
    struct Node *next;
    struct Node *left;
    struct Node *right;
    char *hex;
    char *hcode;
} Node;

void Huff_insert(Node **head, char character, char *hcode);
void LinkedList_beginsert(Node **head, char inchar, int intput);
void LinkedList_midinsert(Node *node, char inchar, int intput);
void LinkedList_sortinsert(Node **head, char inchar, int intput); 
void LinkedList_sortSuper(Node **head, Node *snode);
void LinkedList_remove(Node **head);
void Huff_free(Node *head);
void LinkedList_print(Node *linkedlist);

#endif
