/* Roy Vicerra 
 * CS 357-01: Asgn1
 */

#include <stdio.h>
#define COLUMNS 8		/* Number of columns per tab */

int main() {
	int c, i;		/* c: char from stdin; i: iterator */	
	int ptr = 0;		/* "pointer" of stream position */
	int curr = COLUMNS;	/* number of spaces until tab stop */

	while ((c = getchar()) != EOF) {
		switch(c) {
			case '\t':   /* add spaces according to ptr location */
				if (ptr >= COLUMNS) {
					curr = COLUMNS - (ptr % COLUMNS);
					if (curr == 0) { curr = COLUMNS; }
				}
				else if (ptr > 0 && ptr < COLUMNS) { 
					curr = COLUMNS - ptr; 
				}
				else if (ptr == 0) { curr = COLUMNS; }
				
				for (i = 0; i < curr; i++) {
					putchar(' ');
					++ptr;
				}
				break;

			case '\n':	/* reset ptr */
				putchar(c);
				ptr = 0;
				break;
			
			case '\r':	/* reset ptr */
				putchar(c);
				ptr = 0;
				break;

			case '\b':	/* decrement ptr */
				putchar(c);
				if (ptr > 0) { --ptr; }
				break;

			default:	/* continously increment ptr */
				putchar(c);
				++ptr;
				break; 
		}
	}

	printf("Closing");	
	return 0;
}


