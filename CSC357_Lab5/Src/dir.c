#include <limits.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "dir.h"

char *mypwd(char buffer[]) {
    DIR *directory;
    struct dirent *entry;
    struct stat currdir, fileInfo;
    char *pathname, *buf;
    ino_t ino;
    dev_t dev;
    int cnt = 0;

    /* Retrieve data of current directory */
    if (chdir(buffer) == -1) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    lstat(buffer, &currdir);
    ino = currdir.st_ino;
    dev = currdir.st_dev;
   

    /* Open parent directory and retrieve its data */
    if ((directory = opendir("..")) == NULL) {
        perror("mypwd");
        exit(EXIT_FAILURE);
    }
 
    if (chdir("..") == -1) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

 
    /* Search for child directory name */
    while ((entry = readdir(directory)) != NULL) {
        cnt++;
        if (stat(entry->d_name, &fileInfo) == 0) {
            if (ino == fileInfo.st_ino && dev == fileInfo.st_dev) {
                if (cnt > 1) {
                    /* Write directory name */
                    pathname = (char*) malloc(sizeof(char) * PATH_MAX);
                    memset(pathname, 0, PATH_MAX);
                    buf = mypwd(".");
                    if (strlen(pathname) + strlen(buf) + 
                        strlen(entry->d_name) + 1 > PATH_MAX) {
                        printf("path too long\n");
                    }                
                    strcat(pathname, buf);
                    strcat(pathname, "/");
                    strcat(pathname, entry->d_name);
                    
                    /* Close Directory */
                    if (closedir(directory)) {
                        perror("mypwd");
                        exit(EXIT_FAILURE);
                    }

                    return pathname;

                } else {
                    /* Base Case: Root Directory */
                    return "";
                }            
            }
        } else {
            perror("lstat");
        }
    }

    
    /* In case no directory was found */
    printf("cannot get current directory\n");

   
    /* Close parent directory for safety */
    if (closedir(directory) == -1) {
        perror("mypwd");
        exit(EXIT_FAILURE);
    }
    
    return "";
}

