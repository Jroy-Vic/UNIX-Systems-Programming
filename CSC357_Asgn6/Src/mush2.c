#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>
#include <mush.h>
#define FLAGS 0
#define ARGCNT_FILE 2
#define ARG_STDIN 1
#define ARG_FILE 1
#define FILENAME 0
#define STAGE1 0
#define COM1 0
#define COM2 1
#define COM_DIR 2
#define COM_HDIR 1
#define PIPE_ENDS 2
#define READ_END 0
#define WRITE_END 1
#define PERM 0666
#define CHAR_TRUNC 2

void handler(int signum) {
    /* Does nothing */
    ;
}


int main(int argc, char *argv[]) {
    /* Initialize */
    char *argLine;
    FILE *inFile;
    struct sigaction sa;
    pipeline pipeLine;
    char *hdir_path;
    uid_t uid;
    struct passwd *password;
    pid_t *childProc;
    int (*pipefd)[PIPE_ENDS];
    int idx, pipeidx, endidx, childCnt, pipeCnt;
    int infd, outfd;   
    int status, statusInt;
    int len;
    char *fileName;

    /* Initialize signal handler */
    sa.sa_handler = handler;
    sa.sa_flags = FLAGS;
    sigemptyset(&(sa.sa_mask));
    if ((sigaction(SIGINT, &sa, NULL)) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }


/* ~~~~~~~~~~~~~~~~~~~~~  Handle Command Line Input  ~~~~~~~~~~~~~~~~~~~ */


    /* Handle Command Line Input */
    /* If file is given, read command from file */
    if (argc == ARGCNT_FILE) {
        if (!(inFile = fopen(argv[ARG_FILE], "r"))) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    /* If no file is given, read from stdin */
    } else if (argc == ARG_STDIN) {
        inFile = stdin;
    /* Arguments should not exceed 2 */
    } else {
        fprintf(stderr, "only one file permitted!\n"\
            "usage: %s < file >\n", argv[FILENAME]);
        exit(EXIT_FAILURE);
    }


/* ~~~~~~~~~~~~~~~~~~~~~  Handle Shell Commands  ~~~~~~~~~~~~~~~~~~~~~~~~ */


    /* Handle Shell Commands */
    /* Poll for commands until user signals EOF */
    while (!(feof(inFile))) {
        /* Print Shell Prompt if both stdin and stdout are ttys */
        if (isatty(fileno(inFile)) == -1) {
            perror("isatty");
            exit(EXIT_FAILURE);
        } else {
            if (isatty(fileno(stdout)) == -1) {
                perror("isatty");
                exit(EXIT_FAILURE);
            } else {
                printf("8-P ");
                if ((fflush(stdout)) == EOF) {
                    perror("fflush");
                    exit(EXIT_FAILURE);
                }
            }
        }

        /* Parse Shell Command */
        if (!(argLine = readLongString(inFile))) {
            /* readLongString returns Null when EOF hit or 
                * on error (kinda annoying to be honest) */

            /* If EOF is hit while parsing (user hit enter), 
                * continue polling for more commands */
            if (feof(inFile)) {
                continue;
            /* If caused by signal */
            } else if (errno == EINTR) {
                printf("\n");
                if(fflush(stdout) == -1) {
                    perror("fflush");
                    exit(EXIT_FAILURE);
                }
                continue;
            }
            /* If caused by error, terminate */
            else {
                perror("readLongString");
                exit(EXIT_FAILURE);
            }
        }
    
        if (!(pipeLine = crack_pipeline(argLine))) {
            /* Checking whether user input was empty or if 
                * an error occurred */
            
            /* If error */
            if (ferror(inFile)) {
                perror("crack_pipeline");
                exit(EXIT_FAILURE);
            /* If empty line */
            } else {
                continue;
            }
        }


        /* Handle and execute Shell commands */
        /* Check for "cd" command */
        if (strcmp(pipeLine->stage[STAGE1].argv[COM1], "cd") == 0) {
            /* Check if cd is paired with directory name, or if too many args */
            
            /* Directory name inputted */
            if (pipeLine->stage[STAGE1].argc == COM_DIR) {
                if (chdir(pipeLine->stage[STAGE1].argv[COM2]) == -1) {
                    perror("chdir");
                    exit(EXIT_FAILURE);
                }

            /* If no directory inputted, change to Home Directory */
            } else if (pipeLine->stage[STAGE1].argc == COM_HDIR) {
                /* Get Home Directory path from HOME environment 
                    * or use user's passwd entry */
                if ((uid = getuid()) == -1) {
                    perror("getuid");
                    exit(EXIT_FAILURE);
                }
                if (!(password = getpwuid(uid))) {
                    perror("getpwuid");
                    exit(EXIT_FAILURE);
                }

                if ((hdir_path = getenv("HOME")) || 
                    (hdir_path = password->pw_dir)) {
                    if (chdir(hdir_path) == -1) {
                        perror("chdir");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    fprintf(stderr, "unable to determine home directory\n");
                    exit(EXIT_FAILURE);
                }
                
            /* Incorrect usage */
            } else {
                fprintf(stderr, "usage: cd < directory >\n");
            }

            /* Poll for next Shell command */
            continue;
        }
     

/* ~~~~~~~~~~~~~~~~~~~~~~~  Create Child Processes  ~~~~~~~~~~~~ */   


        /* Create Child Processes */
        childCnt = pipeLine->length;
        if (!(childProc = (pid_t*) malloc(childCnt * sizeof(pid_t)))) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }


        /* Create Pipes between each child process */
        pipeCnt = (pipeLine->length) - 1;
        if (!(pipefd = malloc(pipeCnt * (sizeof(int) * 2)))) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        for (idx = 0; idx < pipeCnt; idx++) {
            /* Create each Pipe */
            if (pipe(pipefd[idx]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }


        /* Fork each process */
        for (idx = 0; idx < childCnt; idx++) {
            if ((childProc[idx] = fork()) == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            /* Child Process */
            if (!childProc[idx]) {
                /* Signal does not need to be forwarded to child */
                /* Change handler back to default */
                sa.sa_handler = SIG_DFL;
                if ((sigaction(SIGINT, &sa, NULL)) == -1) {
                    perror("sigaction");
                    exit(EXIT_FAILURE);
                }


                /* Handling Read End of Pipe (INPUT) */
                /* If no specified input, use STDIN */
                if (!(pipeLine->stage[idx].inname)) {
                    /* Connecting Read End to STDIN; if first stage, 
                        * take from original STDIN */
                    /* Remember pipes have one less count than childCnt */
                    if (idx > 0) {
                        if (dup2(pipefd[(idx - 1)][READ_END],
                         STDIN_FILENO) == -1) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                /* If a specified file, redirect Read End to file */
                } else {
                    if ((infd = open(pipeLine->stage[idx].inname,
                         O_RDONLY)) == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }

                    if (dup2(infd, STDIN_FILENO) == -1) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }

                /* Handling Write End of Pipe (OUTPUT) */
                /* If no specified output, use STDOUT */
                if (!pipeLine->stage[idx].outname) {
                    /* Connecting Write End to STDOUT; if last stage,
                         *  take from original STDOUT */
                    if (idx < pipeCnt) {
                        if (dup2(pipefd[idx][WRITE_END], STDOUT_FILENO) == -1) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                /* If a specified file, redirect Write End to file */
                } else {
                    /* Outputting contents into file, 
                         * must have proper permissions */
                    if ((outfd = open(pipeLine->stage[idx].outname,
                         O_WRONLY | O_TRUNC | O_CREAT, PERM)) == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    
                    if (dup2(outfd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }

                
                /* After pipes are used, dispose of them (including ends) */
                for (pipeidx = 0; pipeidx < pipeCnt; pipeidx++) {
                    for (endidx = 0; endidx < PIPE_ENDS; endidx++) {
                        /* Check if pipe exists */
                        if (pipefd[pipeidx][endidx]) {
                            if (close(pipefd[pipeidx][endidx]) == -1) {
                                perror("close");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                }

                
                /* One file system is setup, execute each command */
                if (execvp(pipeLine->stage[idx].argv[COM1],
                     pipeLine->stage[idx].argv) == -1) {
                    fileName = pipeLine->stage[idx].argv[COM1];
                    len = strlen(fileName);
                    if (len >= CHAR_TRUNC) {
                        /* Remove "./" */
                        memmove(fileName, fileName + CHAR_TRUNC, len - 1);
                        fileName[len - CHAR_TRUNC] = '\0';
                    }
                    perror(fileName);
                    exit(EXIT_FAILURE);
                }
            }
        }


// ~~~~~~~~~~~~~~~~~~~~~~~  Handle Parent  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


        /* After iterating through, the last process is the parent */
        /* Close all open pipes in parent process */
        for (pipeidx = 0; pipeidx < pipeCnt; pipeidx++) {
            for (endidx = 0; endidx < PIPE_ENDS; endidx++) {
                /* Check if pipe exists */
                if (pipefd[pipeidx][endidx]) {
                    if (close(pipefd[pipeidx][endidx]) == -1) {
                        perror("close");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }


        /* Wait for all child processes to terminate */ 
        while ((statusInt = wait(&status)) != 0) {
            /* If interrupted by signal */
            if (errno == EINTR) {
                printf("\n");
                if (fflush(stdout) == -1) {
                    perror("fflush");
                    exit(EXIT_FAILURE);
                }
                break;       
            }
            /* If there are no more processes running, break out of loop */
            if (statusInt == -1) {
                //perror("wait");
                break;
            }
            /* Status of child processes (used to debug) */
            /* if (WIFEXITED(status)) {
                printf("Child process terminated with status: %d\n",
                    WEXITSTATUS(status));
                if (fflush(stdout) == EOF) {
                    perror("fflush");
                    exit(EXIT_FAILURE);
                }
            } else {
                printf("Child process terminated abnormally\n");
                if (fflush(stdout) == EOF) {
                    perror("fflush");
                    exit(EXIT_FAILURE);
                }
            } */
        }


        /* Free all elements */
        free_pipeline(pipeLine);
        free(childProc);
        free(pipefd);
        free(argLine);
    }

    /* Terminate Program with EOF */
    printf("^D\n");
    if ((fflush(stdout)) == EOF) {
            perror("fflush");
            exit(EXIT_FAILURE);
    }

    /* To avoid Valgrind on my behind */
    yylex_destroy();

    
    return 0;
}
