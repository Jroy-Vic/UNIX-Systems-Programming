#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "tar.h"
#define ARGS_OPT 1
#define ARGS_TARNAME 2
#define ARGS_FILESTART 3
#define CFLAG 0
#define TFLAG 1
#define XFLAG 2
#define VFLAG 3
#define SFLAG 4
#define FFLAG 5
#define MAGIC "ustar"

void printTar (Tar *tar) {
    int idx;

    printf("%s\n", tar->tarName);
    printf("options -> c: %d, t: %d, x: %d\n", tar->option[CFLAG], tar->option[TFLAG], tar->option[XFLAG]);
    printf("fileCnt: %d\n", tar->fileCnt);
    printf("Files: \n");
    for (idx = 0; idx < tar->fileCnt; idx++) {
        printf("\t%s\n", tar->files[idx]);
    }
}


int main(int argc, char *argv[]) {
    /* Initialize */
    int idx, argCnt = 0,fileCnt = 0;
    Tar *mytar = (Tar*) calloc(1, sizeof(Tar));
    if (!mytar) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    mytar->tarName = NULL;



    /* Handle Command Line Arguments */
    if (argc == 1) {
        printf("Insufficient number of arguments\n");
        exit(EXIT_FAILURE);
    }
    
    argCnt = strlen(argv[ARGS_OPT]);
    for (idx = 0; idx < argCnt; idx++) {
        if (argv[ARGS_OPT][idx] == 'c' || mytar->option[CFLAG]) {
            mytar->option[CFLAG] = 1;
        } else {
            mytar->option[CFLAG] = 0;
        }
        
        if (argv[ARGS_OPT][idx] == 't' || mytar->option[TFLAG]) {
            mytar->option[TFLAG] = 1;
        } else {
            mytar->option[TFLAG] = 0;
        }

        if (argv[ARGS_OPT][idx] == 'x' || mytar->option[XFLAG]) {
            mytar->option[XFLAG] = 1;
        } else {
            mytar->option[XFLAG] = 0;
        }
     
        if (argv[ARGS_OPT][idx] == 'v' || mytar->option[VFLAG]) {
            mytar->option[VFLAG] = 1;
        } else {
            mytar->option[VFLAG] = 0;
        }
        
        if (argv[ARGS_OPT][idx] == 'S' || mytar->option[SFLAG]) {
            mytar->option[SFLAG] = 1;
        } else {
            mytar->option[SFLAG] = 0;
        }
        
        if (argv[ARGS_OPT][idx] == 'f' || mytar->option[FFLAG]) {
            /* Check if there is a file given after -f */
            if ((mytar->tarName = argv[ARGS_TARNAME]) == NULL) {
                printf("Missing file name\n");
                exit(EXIT_FAILURE);    
            }
            mytar->option[FFLAG] = 1;
        } else {
            mytar->option[FFLAG] = 0;
        }
    }

    /* Check if options argument was valid */
    if ((mytar->option[CFLAG] + mytar->option[TFLAG] + mytar->option[XFLAG]) != 1 || mytar->option[FFLAG] != 1) {
        printf("Invalid argument\n");
        exit(EXIT_FAILURE);
    }

    /* Store all inputted files for archive creation */
    fileCnt = argc - ARGS_FILESTART;
    mytar->fileCnt = fileCnt;

    /* Check if there are any files to archive */
    if (fileCnt > 0) {
        mytar->files = (char**) malloc(fileCnt * sizeof(char*));
        if (!(mytar->files)) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        /* Malloc space for total possible headers */
        mytar->headerCnt = 0;

    /* If -f but no files, exit */
    } else if (mytar->option[CFLAG]) {
        printf("No files were given");
        exit(EXIT_FAILURE);
    }

    /* Store file paths */
    if (mytar->option[CFLAG]) {
        fileCnt = 0;
        for (idx = ARGS_FILESTART; idx < argc; idx++) {
            /* Account for '\0' */
            mytar->files[fileCnt] = (char*) malloc((strlen(argv[idx]) + 1) * sizeof(char)); 
            strcpy(mytar->files[fileCnt], argv[idx]);
            fileCnt++;
        }
    }



    /* Create Tar */
    if (mytar->option[CFLAG]) {
        Tar_create(mytar);        
    }    

    /* Debug */
    printTar(mytar);



    /* Free Tar */
    /* Free File Names */
    for (idx = 0; idx < mytar->fileCnt; idx++) {
        if (mytar->files[idx]) {
            free(mytar->files[idx]);
        }
    }
    /* Free File Names Pointer */
    if (mytar->files) {
        free(mytar->files);
    }
    /* Free Tar */
    if (mytar) {
        free(mytar);
    }



    return 0;
}
