#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "htable.h"
#include "tree.h"
#include "link.h"

#define BUFFER 256
#define FILE_BUFFER 4096

Node *hsearch(Node *root, char encoding[], int idx, Node **hcodes) {
    if (!root) {
        return *hcodes;
    }
    
    if (!(root->left) && !(root->right)) {
        Huff_insert(hcodes, root->character, encoding); 
        return *hcodes;
    }
    
    /* Traverse left branch */
    encoding[idx] = '0';
    encoding[idx + 1] = '\0';
    hsearch(root->left, encoding, idx + 1, hcodes);

    /* Traverse right branch */
    encoding[idx] = '1';
    encoding[idx + 1] = '\0';
    hsearch(root->right, encoding, idx + 1, hcodes);

    return *hcodes;
}


void Huff_encode(int *infd,int *outfd) {
    /* Initialize */
    char encoding[BUFFER];
    /* char **codes[BUFFER]; */
    uint8_t *hencodes;
    unsigned char filebuff[FILE_BUFFER];
    size_t rdbytes, fdidx;
    Node *linkedlist = NULL, *hcodes = NULL, *tmp = NULL;
    int idx = 0, charidx = 0, uniqchar = 0, testfd;
    /* int i; */
    int *characters = (int*) calloc(BUFFER, sizeof(int));
    if (!characters) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }


    
    /* Create characters array */  
    while ((rdbytes = read(*infd, filebuff, sizeof(filebuff))) > 0) { 
        for (fdidx = 0; fdidx < rdbytes; fdidx++) {
            charidx = (unsigned char) filebuff[fdidx];
            if (characters[charidx] == 0) {
                characters[charidx] = 1;
                uniqchar++;
            } else {
                characters[charidx]++;
            }
        }
    } 



    /* Create sorted LinkedList */  
    for (idx = 0; idx < 256; idx++) {
        if (characters[idx]) {
            LinkedList_sortinsert(&linkedlist,
             (unsigned char) idx, characters[idx]);          
       }
    }
    


    /* Create Huffman Tree from LinkedList */
    while (linkedlist && linkedlist->next) {
        Node *superNode = Tree_SuperNode(linkedlist, linkedlist->next);       
        LinkedList_remove(&linkedlist);
        LinkedList_remove(&linkedlist);
        LinkedList_sortSuper(&linkedlist, superNode);
    }


    
    /* Create LinkedList of Huffman Codes */
    encoding[0] = '2';
    hcodes = hsearch(linkedlist, encoding, 0, &hcodes);
    tmp = hcodes;
 

    /* Make Dictionary */
    /*for (i = 0; i < BUFFER; i++) {
        char *character = (char*) malloc(sizeof(char) * BUFFER);
        codes[i] = character;
    }   

    while (tmp) {
        free(codes[tmp->count]);
        codes[tmp->count] = tmp->hcode;
    } */


    /* Write Header Byte */
    hencodes = (uint8_t*) malloc(sizeof(uint8_t) * (1 + (uniqchar * 5)));
    if (!hencodes) {
        perror("malloc");
        exit(EXIT_FAILURE);
    } 
    idx = 0;
 
    hencodes[0] = (uint8_t) (uniqchar - 1);
    printf("Head: 0x%02x\n", hencodes[idx]);
    idx++;
    
    while (tmp) {
        /* Huffman Code */
        hencodes[idx] = (unsigned char) tmp->count; 
        /* (uint8_t) strtol(tmp->hcode, NULL, 2); */
        printf("Code: 0x%02x  |  ", (unsigned int) hencodes[idx]);
        idx++;

        /* Character Frequency */
        hencodes[idx] = (((uint32_t) characters[tmp->count]) >> 24) & 0xFF;
        printf("Count: 0x%02x",hencodes[idx]);

        hencodes[idx + 1] = (((uint32_t) characters[tmp->count]) >> 16) & 0xFF; 
        printf("%02x",hencodes[idx + 1]);

        hencodes[idx + 2] = (((uint32_t) characters[tmp->count]) >> 8) & 0xFF;
        printf("%02x",hencodes[idx + 2]);

        hencodes[idx + 3] = (((uint32_t) characters[tmp->count]) & 0xFF);
        printf("%02x\n",hencodes[idx + 3]);

        idx += 4;

        tmp = tmp->next;
    }
   
    /* Build Body */
    /*tmp = hcodes;
    lseek(*infd, 0, SEEK_SET); 
    while ((rdbytes = read(*infd, filebuff, sizeof(filebuff))) > 0) { 
        for (fdidx = 0; fdidx < rdbytes; fdidx++) {
            charidx = (unsigned char) filebuff[fdidx];
            hencodes = (uint8_t*) realloc(hencodes, 1);
            hencodes[idx] = (uint8_t) strtol(codes[charidx], NULL, 2);
            idx++;
        }
   
    } */  


    testfd = write(*outfd, hencodes, sizeof(uint8_t) * idx);
    printf("%d",testfd);
    if (testfd == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    printf("\n");
 
    tmp = hcodes;
    while (tmp) { 
        printf("0x%s: %s\n", tmp->hex, (tmp->hcode) ? tmp->hcode : "");  
        tmp = tmp->next;
    }


   /* Free Characters */
    free(characters); 

    
    /* Free Huffman codes */
    Huff_free(hcodes);

    /* Free Huffman Tree */
    Tree_free(linkedlist);
    
    /* Free binary encoding */
    free(hencodes);
}
