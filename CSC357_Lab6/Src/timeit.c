#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#define MIN_ARGS 2
#define TICK 0
#define TOCK 1
#define msgs 2
#define DECIMAL 10
#define HALFSEC 500000
#define FLAGS 0

void handler(int signum) {
/* Does nothing, advances through pause() */
;
}


int main (int argc, char *argv[]) {
    /* initialize */
    int sec, sel = TICK;
    struct itimerval it;
    struct sigaction sa;
    char *end, *output[msgs];
   
    /* Handle command line argument */
    if (argc > MIN_ARGS) {
        fprintf(stderr, "Error: Too many arguments\n");
        fprintf(stderr, "Usage: %s (Number of seconds)\n", argv[0]);
        exit(EXIT_FAILURE);
    } else if (argc != MIN_ARGS) {
        fprintf(stderr, "Error: Input number of seconds\n");
        fprintf(stderr, "Usage: %s (Number of seconds)\n", argv[0]);
        exit(EXIT_FAILURE);
    }
     else {
        /* Convert argument to decimal number */
        sec = strtol(argv[1], &end, DECIMAL);
        /* Check if argument is a number (if *end is NULL) */
        if (*end) {
            fprintf(stderr, "Error: Argument is not a number\n");
            fprintf(stderr, "Usage: %s (Number of seconds)\n", argv[0]);
            exit(EXIT_FAILURE);
        } else if (sec == 0) {
            fprintf(stderr, "Error: Input must be greater than 0\n");
            fprintf(stderr, "Usage: %s (Number of seconds)\n", argv[0]);
            exit(EXIT_FAILURE);
        /* Check if argument is positive */
        } else if (sec < 0) {
            fprintf(stderr, "Error: Argument cannot be negative\n");
            fprintf(stderr, "Usage: %s (Number of seconds)\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /* Store messages */
    output[TICK] = "Tick...";
    output[TOCK] = "Tock\n";


    /* Must initialize signal handler before itimer */
    /* Initialize signal handler */
    sa.sa_handler = handler;
    /* Set flags (none) */
    sa.sa_flags = FLAGS;
    /* Empty signal set (Receives all signals) */
    sigemptyset(&(sa.sa_mask));
    /* Set signal to respond to itimer expiration (No previous signal) */
    if ((sigaction(SIGALRM, &sa, NULL)) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
 
    /* Initialize itimer to expire every 0.5 sec */
    /* Set initial expiration time */
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = HALFSEC;
    /* Set proceeding expiration time */
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = HALFSEC;
    /* Set itimer (No previous timer (NULL)) */
    if ((setitimer(ITIMER_REAL, &it, NULL)) == -1) {
        perror("setitimer");
        exit(EXIT_FAILURE);
    }

  
    /* Output Message */       
    while (sec > 0) {
        /* Wait for SIGALRM */
        pause();
        /* Write to stdout after signal */
        if ((write(STDOUT_FILENO, output[sel], strlen(output[sel]))) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        /* Decrement sec after every tock */
        if (sel) {
            sec--;
        }
        /* TOCK if in TICK state, vice versa */
        sel = sel ? TICK : TOCK;
    }


    /* End program */
    if ((write(STDOUT_FILENO, "Time's Up!\n", strlen("Time's Up!\n"))) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    } 

    return 0;
}
