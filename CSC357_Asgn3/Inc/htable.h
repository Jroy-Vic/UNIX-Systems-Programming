#ifndef HTABLE_H
#define HTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "tree.h"
#include "link.h"

#define BUFFER 256
#define FILE_BUFFER 4096

Node *hsearch(Node *root, char encoding[], int idx, Node **hcodes);
void Huff_encode(int *infd, int *outfd);

#endif
