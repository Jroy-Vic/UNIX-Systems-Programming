#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define EXIT_STATUS 0

int main(int argc, char *argv[]) {
    /* Initialize */
    pid_t child_pid, pid;
    int status;

    /* Parent greets world (no need to be unbuffered) */
    printf("Hello, world!\n");


    /* Forks a child process */
    if ((child_pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } 
    /* Get current pid */
    if ((pid = getpid()) == -1) {
        perror("getpid");
        exit(EXIT_FAILURE);
    }

    /* Print parent pid, unbuffered (parent if child_pid > 0) */
    if (child_pid) {
        printf("This is the parent, pid %d.\n", pid);
        if (fflush(stdout) == EOF) {
            perror("fflush");
            exit(EXIT_FAILURE);
        }
        /* Wait for child process to terminate */
        if (wait(&status) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        /* When child process terminates, print goodbye message, unbuffered */
        if (WIFEXITED(status) && (WEXITSTATUS(status) == EXIT_STATUS)) {
            printf("This is the parent, pid %d, signing off.\n", pid);
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        } else {
            exit(EXIT_FAILURE);
        }
    /* Print child pid, unbuffered (child_pid == 0) */
    } else {
        printf("This is the child, pid %d.\n", pid);
        if (fflush(stdout) == EOF) {
            perror("fflush");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
