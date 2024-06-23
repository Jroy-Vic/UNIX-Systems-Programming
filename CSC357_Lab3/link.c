#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "link.h"
#define HEX_SIZE 3

/* Store Huffman Codes into a LinkedList */
void Huff_insert(Node **head, char character, char *hcode) {
    Node *curr = *head;
  
    Node *newNode = (Node*) malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    newNode->hex = (char*) malloc(sizeof(char) * HEX_SIZE);
    if (!(newNode->hex)) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    if (hcode[0] != '2') {
        newNode->hcode = (char*) malloc(sizeof(char) * (strlen(hcode) + 1));
        if (!(newNode->hcode)) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(newNode->hcode, hcode);
    } else {
        newNode->hcode = NULL;
    }
    
    newNode->count = (unsigned char) character;
    sprintf(newNode->hex, "%02x", (unsigned char) character);


    /* If LinkedList is empty or entry has a smaller count or character */
    if (!(*head) || (*head && ((*head)->count > (unsigned char) character))) {
        newNode->next = *head;
        *head = newNode;
        return;
    }

    /* Checks for count; if equal, check characters */
    while (curr->next) {
        if ((curr->next)->count > (unsigned char) character) {
            break;
        }
        curr = curr->next;
    }

    /* If greater than all other entries, place at end */
    newNode->next = curr->next;
    curr->next = newNode;
}


/* Add entry to beginning of LinkedList */
void LinkedList_beginsert(Node **head, char inchar, int intput) {
    Node *newNode = (Node*) malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    newNode->character = inchar;
    newNode->count = intput;   
    newNode->next = *head;
    newNode->left = NULL;
    newNode->right = NULL;

    *head = newNode;
} 


/* Add entry after a specific node in LinekdList */
void LinkedList_midinsert(Node *node, char inchar, int intput) {
    Node *newNode = (Node*) malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    newNode->character = inchar;
    newNode->count = intput;   
    newNode->next = node->next;
    newNode->left = NULL;
    newNode->right = NULL;

    node->next = newNode;
}


/* Insert entry with sort */
void LinkedList_sortinsert(Node **head, char inchar, int intput) {
    Node *curr = *head;

    /* If LinkedList is empty or entry has a smaller count or character */
    if (!(*head) || (((*head)->count > intput) && 
        ((*head)->character == '\0')) || ((*head)->count > intput) || 
        (((*head)->count == intput) && ((*head)->character > inchar))) {
        LinkedList_beginsert(head,inchar,intput);
        return;
    }

    /* Checks for count; if equal, check characters */
    while (curr->next) {
        if ((curr->next)->count > intput) {
            break;
        } else if (((curr->next)->count == intput) && 
                (((curr->next)->character == '\0') || 
                ((curr->next)->character > inchar))) {
            break;
        }
        curr = curr->next;
    }

    /* If greater than all other entries, place at end */
    LinkedList_midinsert(curr,inchar,intput);
}


/* Insert superNode into LinkedList */
void LinkedList_sortSuper(Node **head, Node *snode) {
    Node *curr = *head;

    /* If LinkedList is empty or entry has a smaller count or character, 
 * place in the front of the list. If equal, place after node */
    if (!(*head) || ((*head)->count > snode->count) || ((*head)->count == snode->count)) {
        snode->next = *head;
        *head = snode;
        return;
    } 

    /* Checks for count; if equal, places before current node */
    while (curr->next) {
        if ((curr->next)->count > snode->count) {
            break;
        } else if ((curr->next)->count == snode->count) {
            break;
        }
        curr = curr->next;
    }

    /* Place superNode before observed node */
    snode->next = curr->next;
    curr->next = snode;
}


/* Remove first entry from LinkedList (Does not free) */
void LinkedList_remove(Node **head) {
    if (!(*head)) {
        return;
    }   

    *head = (*head)->next;
}


/* Deallocate LinkedList */
void Huff_free(Node *head) {
    Node *curr = head;
    while (curr) {
        Node *tmp = curr;
        curr = curr->next;

        if (tmp->hex) {
            free(tmp->hex);
        }

        if (tmp->hcode) {
            free(tmp->hcode);
        }

        free(tmp);
    }
}


/* Print LinkedList for debugging */
void LinkedList_print(Node *linkedlist) {
    printf("\nStart of LinkedList\n");
    while (linkedlist) {
        printf("Character: %c | Count: %d\n", 
                linkedlist->character, linkedlist->count);
        linkedlist = linkedlist->next;
    }
    printf("End of LinkedList\n\n");
}


