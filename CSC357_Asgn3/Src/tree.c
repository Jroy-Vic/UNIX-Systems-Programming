#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "tree.h"

/* Create TreeNode */
Node *Tree_createNode(char inchar, int intput) {
    Node *tnode = (Node*) malloc(sizeof(Node));
    if (!tnode) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    tnode->character = inchar;
    tnode->count = intput;
    tnode->next = NULL;
    tnode->left = NULL;
    tnode->right = NULL;
    
    return tnode;
}


/* Adds superNode to tree */
Node *Tree_addSuper(Node *input) {
    Node *snode = (Node*) malloc(sizeof(Node));
    if (!snode) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    snode->character = input->character;
    snode->count = input->character;
    snode->next = NULL;
    snode->left = input->left;
    snode->right = input->right;
    
    return snode;
}


/* Create or Add TreeNode to Tree */
void Tree_insert(Node **rootptr, Node *input) {
    Node *root = *rootptr;
    if (root) {
        /*Tree already has contents */
        if (input->count < root->count) {
            Tree_insert(&(root->left), input);
        } else if (input->count > root->count) {
            Tree_insert(&(root->right), input);
        } else if (input->character == '\0') {
            Tree_insert(&(root->left), input);
        } else if (input->character < root->character) { 
            Tree_insert(&(root->left), input);
        } else {
            Tree_insert(&(root->right), input);
        }
        
    } else {
        /* Empty Tree */
        (*rootptr) = Tree_addSuper(input);
    }
}

/* Create a superNode */
Node *Tree_SuperNode(Node *node1, Node *node2) {
    Node *snode = (Node*) malloc(sizeof(Node));
    if (!snode) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    snode->character = '\0';
    snode->count = node1->count + node2->count;
    snode->next = NULL;
    snode->left = node1;
    snode->right = node2;

    return snode;
} 


/* Free TreeNodes */
void Tree_free(Node *root) {
    if (!root) {
        return;
    }

    Tree_free(root->left);
    Tree_free(root->right);
    free(root);
}


/* Print Tree for debugging */
void printtabs(int num) {
    int i;
    for (i = 0; i < num; i++) {
        printf("\t");
    }
}

void Tree_print_rec(Node *root, int level) {
    if (root) {
        printtabs(level);
        printf("Node: Character: %c | Count: %d\n",
                root->character, root->count);
        printtabs(level);
        printf("Left:\n");
        Tree_print_rec(root->left, level+1);
        printtabs(level);
        printf("Right:\n");
        Tree_print_rec(root->right, level+1);
    } else {
        printtabs(level);
        printf("---<empty>---\n");
    }
}

void Tree_print(Node *root) {
    printf("Start of Binary Tree\n");
    Tree_print_rec(root, 0);
    printf("End of Binary Tree\n\n");
}

