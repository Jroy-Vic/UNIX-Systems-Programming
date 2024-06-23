#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define ARGS 2
#define PROGRAM 0
#define COMMAND 1
#define EXIT_STATUS 0

int main(int argc, char *argv[]) {
    /* Initialize */
    char *program;
    pid_t child_pid;
    int status;

    /* Handle command line arguments */
    if (argc == ARGS) {
        program = argv[COMMAND];
    } else {
        /* Write usage error message (does not need to be buffered) */
        fprintf(stderr, "usage: %s command\n", argv[PROGRAM]);
        exit(EXIT_FAILURE);
    }

    
    /* Fork a child process */
    if ((child_pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    /* Parent: waits for child to terminate, prints its exit status */
    if (child_pid) {
        if (wait(&status) == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        /* Print exit status of child process (unbuffered) */
        /* Child exited successfully */
        if (WIFEXITED(status) && (WEXITSTATUS(status) == EXIT_STATUS)) {
            printf("Process %d succeeded.\n", child_pid);
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Process %d exited with an error value.\n", child_pid);
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
    /* Child: execute inputted command */
    } else {
        /* Execute command line argument (take in absolute or relative path) */
        /* execlp usage: filename, argv[0], NULL (indicating end of argv) */
        if (execl(program, program, NULL) == -1) {
            perror(program);
            exit(EXIT_FAILURE);
        }
    }    
    
    return 0;
}
