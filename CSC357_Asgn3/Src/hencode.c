#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "htable.h"
#define FILE_BUFFER 4096


int main(int argc, char *argv[]) {
    /* initialization */
    int infd, outfd; 
    /* int multiplier = 0; */
    /*char filebuff[FILE_BUFFER], *huffinput = NULL;*/
    /* ssize_t rdbytes; */


    /* Argument and File Handling */ 
    if (argc < 2 || argc > 3) {
        exit(EXIT_FAILURE);
    }
   
    infd = open(argv[1], O_RDONLY);
    if (infd == -1) {
        exit(EXIT_FAILURE);
    } 

    if (argc == 3) {
        outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outfd == -1) {
            close(infd);
            exit(EXIT_FAILURE);
        } 
    } else {
        outfd = STDOUT_FILENO;
    }
    
    Huff_encode(&infd, &outfd);
    
    /* Read File */
    /* while ((rdbytes = read(infd, filebuff, sizeof(filebuff))) > 0) {
        if (multiplier == 0) {
            huffinput = (char*) malloc(sizeof(char) * (rdbytes + 1));
        } else {
            huffinput = (char*) realloc(huffinput, (sizeof(char) * 
            ((FILE_BUFFER * multiplier) + rdbytes + 1)));
        }
        
        if (!huffinput) {
            exit(EXIT_FAILURE);
        }
       
        memcpy(huffinput + (FILE_BUFFER * multiplier), filebuff, rdbytes);
        huffinput[(FILE_BUFFER * multiplier) + rdbytes] = '\0';
        multiplier++;
        printf("test\n");    
    }
    
    if (rdbytes == -1) {
        close(infd);
        if (argc == 3) {
            close(outfd);
        }
        exit(EXIT_FAILURE);
    }
*/

    close(infd);

/*    printf("%s\n",huffinput); */

    if (argc == 3) {
        close(outfd);
    }
    
/*    if (huffinput) { 
        free(huffinput);
    } */

    return 0;
}
