#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include "link.h"

Node *Tree_createNode(char inchar, int intput);
Node *Tree_addSuper(Node *input);
void Tree_insert(Node **rootptr, Node *input);
Node *Tree_SuperNode(Node *node1, Node *node2);
void Tree_free(Node *root);
void printtabs(int num);
void Tree_print_rec(Node *root, int level);
void Tree_print(Node *root);
  
#endif
