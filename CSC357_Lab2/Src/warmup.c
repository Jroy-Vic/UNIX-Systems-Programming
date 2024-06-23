#include <stdio.h>
/*char *cp;
char arr[];
char *ap = arr;
int *iptr;
int **pp = iptr;

int *ptr = &ptr;
*/

int main() {
	char s[] = "Hello, world!\n";
	char *p;

	for (p = s; p != '\0'; p++) { putchar(*p); }	
	return 0;
}

