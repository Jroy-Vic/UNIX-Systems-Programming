#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
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


int main(int argc, char *argv[]) {
    /* Initialize */
    char encoding[BUFFER];
    unsigned char filebuff[FILE_BUFFER];
    size_t rdbytes, i;
    FILE *file;
    Node *linkedlist = NULL, *hcodes = NULL, *tmp = NULL;
    int idx = 0, charidx = 0;
    int *characters = (int*) calloc(BUFFER, sizeof(int));
    if (!characters) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    

    /* Check for file */
    if (argc > 1) {
        file = fopen(argv[1], "rb");
        if (!file) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }
    } else {
        file = stdin;
    }

    
    /* Create characters array */  
    while ((rdbytes = fread(filebuff, 1, sizeof(filebuff), file)) > 0) { 
        for (i = 0; i < rdbytes; i++) {
            charidx = (unsigned char) filebuff[i];
            if (characters[charidx] == 0) {
                characters[charidx] = 1;
            } else {
                characters[charidx]++;
            }
        }
    } 

    /* Close file when finished */ 
    if (file != stdin) {
        fclose(file);
    }



    /* Create sorted LinkedList */  
    for (idx = 0; idx < 256; idx++) {
        if (characters[idx]) {
            LinkedList_sortinsert(&linkedlist, (unsigned char) idx, characters[idx]);          
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
    while (tmp) {
        printf("0x%s: %s\n", tmp->hex, (tmp->hcode) ? tmp->hcode : "");
        tmp = tmp->next;
    }


    
    /* Free characters array when finished */
    free(characters); 
    
    /* Free Huffman codes */
    Huff_free(hcodes);

    /* Free Huffman Tree */
    Tree_free(linkedlist);
    
    return 0;
}
