#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <netdb.h>
#include <pwd.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <talk.h>
#define MIN_ARG 2
#define MAX_ARG 7
#define HOST_PORT 2  /* Number of arguments that are not flags */
#define HOSTNAME 0
#define PORT 1
#define DECIMAL 10
#define MAXLEN 1000
#define HOSTNAME_LEN 1000
#define MODE_SERVER "server"
#define MODE_CLIENT "client"
#define DEFAULT_BACKLOG 100 /* Can listen up to 100 clients */
#define CONFIRM_LEN 5
#define CONFIRM_MSG "ok"
#define UNCONFIRM_MSG "not ok"
#define LOCAL 0
#define REMOTE 1
#define SAFLAGS 0


volatile sig_atomic_t exit_flag = 0;

void handler(int signum) {
    /* Handle SIGQUIT signal */
    exit_flag++;
}


int main(int argc, char *argv[]) {
    /* Initialize */
    char *hostname, init_hostname[HOSTNAME_LEN], *endptr;
    int port;
    int vflag = 0, aflag = 0, Nflag = 0;
    char *args[HOST_PORT], confirm[CONFIRM_LEN];
    int idx, args_idx = 0, vCnt = 0, aCnt = 0, NCnt = 0;
    int msgLen, sockfd, connected_sockfd;
    struct sockaddr_in sockAddr, connected_sockAddr, clientAddr;
    socklen_t addrlen;
    char localaddr[INET_ADDRSTRLEN], 
         clientaddr[INET_ADDRSTRLEN], buf[MAXLEN + 1];
    struct hostent *hostent;
    uid_t clientUID;
    struct passwd *pw;    
    int done = 0, stdIN_len;
    struct pollfd fds[REMOTE + 1];
    struct sigaction sigAct;

    /* Handle Argument Line Inputs */
    if (argc >= MIN_ARG) {
        if (argc < MAX_ARG) {
            for (idx = 1; idx < argc; idx++) {
                if (strcmp(argv[idx], "-v") == 0) {
                    vflag = 1;
                    vCnt++;
                } else if (strcmp(argv[idx], "-a") == 0) {
                    aflag = 1;
                    aCnt++;
                } else if (strcmp(argv[idx], "-N") == 0) {
                    Nflag = 1;
                    NCnt++;
                } else {
                    args[args_idx++] = argv[idx];
                }
            }
            /* Store args to port and/or hostname */
            if (args_idx == HOST_PORT) {
                hostname = args[HOSTNAME];
                port = strtol(args[PORT], (char**) &endptr, DECIMAL);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid port (must be an integer)\n");
                    fprintf(stderr, "Usage: %s [ -v ] [ -a ] [ -N ] "\
                        "[ hostname ] port\n", argv[0]);    
                    exit(EXIT_FAILURE);
                }
            } else if (args_idx == PORT) {
                port = strtol(args[0], (char**) &endptr, DECIMAL);
                hostname = "(none)";
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid port (must be an integer)\n");
                    fprintf(stderr, "Usage: %s [ -v ] [ -a ] [ -N ] "\
                        "[ hostname ] port\n", argv[0]);    
                    exit(EXIT_FAILURE);
                }
            } else {
                fprintf(stderr, "Invalid arguments\n");
                fprintf(stderr, "Usage: %s [ -v ] [ -a ] [ -N ] "\
                    "[ hostname ] port\n", argv[0]);    
                exit(EXIT_FAILURE);
            }

            /* Check for valid flags */
            if ((vCnt > 1) || (aCnt > 1) || (NCnt > 1)) {
                fprintf(stderr, "Invalid arguments\n");
                fprintf(stderr, "Usage: %s [ -v ] [ -a ] [ -N ] "\
                    "[ hostname ] port\n", argv[0]);    
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "Too many arguments\n");
            fprintf(stderr, "Usage: %s [ -v ] [ -a ] [ -N ] "\
                "[ hostname ] port\n", argv[0]); 
            exit(EXIT_FAILURE);
        }

    } else {
        fprintf(stderr, "No arguments given\n");
        fprintf(stderr, "Usage: %s [ -v ] [ -a ] [ -N ]"\
            " [ hostname ] port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Options:\n  -v: %d\n  -a: %d\n  -N: %d\n  hostname: %s\n  port:"\
        " %d\n", vflag, aflag, Nflag, hostname, port);

    
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


    /* Handle Server */
    if (args_idx == PORT) {
        printf("  mode: %s\n", MODE_SERVER); 
        /* Create socket for server with TCP */
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("Server: socket created\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }

        /* Initialize socket input information */
        sockAddr.sin_family = AF_INET;
        /* Port and Socket Address must be converted 
         * to network-byte order */
        sockAddr.sin_port = htons(port);
        /* Accepts any connecting addresses */
        sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        /* Open server to client */
        if (bind(sockfd, (struct sockaddr*) &sockAddr, 
          sizeof(sockAddr)) == -1) {
            perror("bind");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("Server: binded and is available to client\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }

        /* Listen for confirmation */
        if (vflag) {
            printf("Server: listening for incoming connections...");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
        if (listen(sockfd, DEFAULT_BACKLOG) == -1) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        
        /* Accept connection */
        addrlen = sizeof(connected_sockAddr);
        if ((connected_sockfd = accept(sockfd, (struct sockaddr*) &clientAddr,
             &addrlen)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("connection accepted\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }

        /* Begin communication with client */
        /* Get additional address info of client */
        /* Update connected_sockAddr len, store local address
         * info into connected_sockAddr */
        addrlen = sizeof(connected_sockAddr);
        if (getsockname(connected_sockfd, 
            (struct sockaddr*) &connected_sockAddr, &addrlen) == -1) {
                perror("getsockname");
                exit(EXIT_FAILURE);
        }
        if (!(inet_ntop(AF_INET, 
            (uint32_t*) &connected_sockAddr.sin_addr.s_addr,
            localaddr, sizeof(localaddr)))) {
                perror("inet_ntop");
                exit(EXIT_FAILURE);
        }
        if (!(inet_ntop(AF_INET, (uint32_t*) &clientAddr.sin_addr.s_addr, 
                 clientaddr, sizeof(clientaddr)))) {
                perror("inet_ntop");
                exit(EXIT_FAILURE);
        }
        /* If -v is on */
        if (vflag) {
            printf("Server: connection has been established"\
            " on server end: %s:%d->%s:%d\n",
                clientaddr, ntohs(clientAddr.sin_port),
                localaddr, ntohs(connected_sockAddr.sin_port));             
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
 

        /* Ask user for confirmation */
        /* Retrieve Client username */
        if ((msgLen = recv(connected_sockfd, buf, sizeof(buf), 0)) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("Server: received Client username\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
        /* Ask server for input, or skip if -a */
        if (aflag) {
            if ((msgLen = send(connected_sockfd, CONFIRM_MSG,
             strlen(CONFIRM_MSG), 0)) == -1) {
                perror("send");
                exit(EXIT_FAILURE);
            }    
        } else { 
            if (getnameinfo((struct sockaddr*) &clientAddr, sizeof(clientAddr), 
                    init_hostname, sizeof(init_hostname), NULL, 0, 0) == -1) {
                perror("getnameinfo");
                exit(EXIT_FAILURE);
            }
            printf("Server: Mytalk request from %s@%s. Accept (y/n)?\n", 
                buf, init_hostname);
            if (fflush(stdout) == EOF) {
                    perror("fflush");
                    exit(EXIT_FAILURE);
            }
            if (read(STDIN_FILENO, &confirm, sizeof(confirm) - 1) == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            /* Send Server confirmation to Client */  
            if ((strncasecmp(confirm, "yes", strlen("yes")) == 0) ||
                 (strncasecmp(confirm, "y", strlen("y")) == 0)) {
                if ((msgLen = send(connected_sockfd, CONFIRM_MSG,
                 strlen(CONFIRM_MSG), 0)) == -1) {
                    perror("send");
                    exit(EXIT_FAILURE);
                }    
            /* Decline confirmation, terminate program */
            } else {
                if ((msgLen = send(connected_sockfd, UNCONFIRM_MSG,
                     strlen(UNCONFIRM_MSG), 0)) == -1) {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                if (vflag) {
                    printf("Server: declining connection\n");
                    if (fflush(stdout) == EOF) {
                        perror("fflush");
                        exit(EXIT_FAILURE);
                    }
                }       
                return 0; 
            }
        }
        
        /* Open ncurses window */
            if (!Nflag) {
                start_windowing();

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    /* Handle Signals */
    sigAct.sa_handler = handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SAFLAGS;
    
    if (sigaction(SIGINT, &sigAct, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

                fds[LOCAL].fd = STDIN_FILENO;
                fds[LOCAL].events = POLLIN;
                fds[LOCAL].revents = 0;
                fds[REMOTE] = fds[LOCAL];
                fds[REMOTE].fd = connected_sockfd;    
                
                do {
                    /* Poll stdin indefinitely */
                    if (poll(fds, (sizeof(fds)/sizeof(struct pollfd)),
                         -1) == -1) {
                        /* If poll is interrupted by signal, continue */
                        if (errno == EINTR) {
                            if ((msgLen = send(connected_sockfd, 
                        "\nConnection closed. ^C to terminate.\n", 
                        strlen("\nConnection closed. ^C to"\
                        " terminate.\n"), 0)) == -1) {
                                perror("send");
                                exit(EXIT_FAILURE);
                            } 
                            break;
                        }
                        perror("poll");
                        exit(EXIT_FAILURE);
                    }
                    if (fds[LOCAL].revents & POLLIN) {
                        if ((stdIN_len = read_from_input(buf, 
                                MAXLEN)) == -1) {
                            perror("read_from_input");
                            exit(EXIT_FAILURE);
                        }
                        /* Check for communication termination (EOF) */
                        if (has_hit_eof()) {
                            done = 1;
                        } else { 
                            if ((msgLen = send(connected_sockfd, buf,
                             stdIN_len, 0)) == -1) {
                                perror("send");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                    if (fds[REMOTE].revents & POLLIN) {
                        if ((msgLen = recv(connected_sockfd, buf,
                             sizeof(buf), 0)) == -1) {
                            perror("recv");
                            exit(EXIT_FAILURE);
                        }
                        if (write_to_output(buf, msgLen) == -1) {
                            perror("write");
                            exit(EXIT_FAILURE);
                        }
                        /* Terminate communication (EOF) */
                        if (msgLen == 0) {
                            done = 1;
                        }
                    }
                } while ((!done) && (!exit_flag));
 
        /* Or communicate through terminal line */
            } else {
                fds[LOCAL].fd = STDIN_FILENO;
                fds[LOCAL].events = POLLIN;
                fds[LOCAL].revents = 0;
                fds[REMOTE] = fds[LOCAL];
                fds[REMOTE].fd = connected_sockfd;    
                
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    /* Handle Signals */
    sigAct.sa_handler = handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SAFLAGS;
    
    if (sigaction(SIGINT, &sigAct, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

                do {
                    /* Poll stdin indefinitely */
                    if (poll(fds, (sizeof(fds)/sizeof(struct pollfd)),
                         -1) == -1) {
                        if (errno == EINTR) {
                            if ((msgLen = send(connected_sockfd, 
                        "\nConnection closed. ^C to terminate.\n", 
                        strlen("\nConnection closed. ^C to"\
                        " terminate.\n"), 0)) == -1) {
                                perror("send");
                                exit(EXIT_FAILURE);
                            } 
                            break;
                        }
                        perror("poll");
                        exit(EXIT_FAILURE);
                    }
                    if (fds[LOCAL].revents & POLLIN) {
                        if ((stdIN_len = read(STDIN_FILENO, 
                           buf, MAXLEN)) == -1) {
                            perror("read");
                            exit(EXIT_FAILURE);
                        }
                        /* Check for communication termination (EOF) */
                        if (stdIN_len == 0) {
                            done = 1;
                        } else { 
                            if ((msgLen = send(connected_sockfd, 
                            buf, stdIN_len, 0)) == -1) {
                                perror("send");
                                exit(EXIT_FAILURE);
                            } 
                        }
                    }
                    if (fds[REMOTE].revents & POLLIN) {
                        if ((msgLen = recv(connected_sockfd,
                             buf, sizeof(buf), 0)) == -1) {
                            perror("recv");
                            exit(EXIT_FAILURE);
                        }
                        if (write(STDOUT_FILENO, buf, msgLen) == -1) {
                            perror("write");
                            exit(EXIT_FAILURE);
                        }
                        /* Terminate communication (EOF) */
                        if (msgLen == 0) {
                            done = 1;
                        }
                    }
                } while ((!done) && (!exit_flag));
            }
       
        /* Close ncurses window if applicable */ 
        if (!Nflag) {
            while ((exit_flag == 0) && (done == 1)) {
                pause();
            } 

            stop_windowing();
        } else {
            while ((exit_flag == 0) && (done == 1)) {
                pause();
            } 
        }

        /* Close socket communication */
        if (vflag) {
            printf("\nServer: closing sockets...");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
        if (close(sockfd) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        if (close(connected_sockfd) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("complete.\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }

        }   
    }


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


    /* Handle Client */
    if (args_idx == HOST_PORT) {
        printf("  mode: %s\n", MODE_CLIENT);
        /* Create socket for client with TCP */
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("Client: socket created\n"); 
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
        /* Identify server to communicate with */
        if (!(hostent = gethostbyname(hostname))) {
            perror("gethostbyname");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("Client: retrieved server address\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }

        /* Initialize socket input information */
        sockAddr.sin_family = AF_INET;
        /* Port and Socket Address must be converted to network-byte order */
        sockAddr.sin_port = htons(port);
        /* Address is the server address (only one address in list) */
        sockAddr.sin_addr.s_addr = *(uint32_t*)(hostent->h_addr_list[0]);
        
        /* Connect to server */
        if (connect(sockfd, (struct sockaddr*) &sockAddr,
          sizeof(sockAddr)) == -1) {
            perror("connect");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("Client: successfully connected to server"\
               "...awaiting confirmation\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }

        /* Obtain username from UID and send to Server */
        clientUID = getuid();
        if (!(pw = getpwuid(clientUID))) {
            perror("getpwuid");
            exit(EXIT_FAILURE);
        } 
        /* Send username without NULL terminator */
        if ((msgLen = send(sockfd, pw->pw_name, 
            (sizeof(pw->pw_name) - 0), 0)) == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("Client: sent username\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
        if (vflag) {
            printf("Client: waiting for response from %s...\n", hostname);
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }

        /* Receive confirmation from Server */
        if ((msgLen = recv(sockfd, buf, sizeof(buf), 0)) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        /* Continue if confirmed, terminate otherwise */
        if (strcmp(buf, CONFIRM_MSG) == 0) {
            /* Open ncurses */
            if (!Nflag) {
                start_windowing();
 
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    /* Handle Signals */
    sigAct.sa_handler = handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SAFLAGS;
    
    if (sigaction(SIGINT, &sigAct, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
               
                fds[LOCAL].fd = STDIN_FILENO;
                fds[LOCAL].events = POLLIN;
                fds[LOCAL].revents = 0;
                fds[REMOTE] = fds[LOCAL];
                fds[REMOTE].fd = sockfd;    
                
                do {
                    /* Poll stdin indefinitely */
                    if (poll(fds, (sizeof(fds)/sizeof(struct pollfd)),
                     -1) == -1) {
                        if (errno == EINTR) {
                            if ((msgLen = send(sockfd, 
                        "\nConnection closed. ^C to terminate.\n", 
                        strlen("\nConnection closed. ^C to"\
                        " terminate.\n"), 0)) == -1) {
                                perror("send");
                                exit(EXIT_FAILURE);
                            } 
                            break;
                        }
                        perror("poll");
                        exit(EXIT_FAILURE);
                    }
                    if (fds[LOCAL].revents & POLLIN) {
                        if ((stdIN_len = read_from_input(buf,
                         MAXLEN)) == -1) {
                            perror("read_from_input");
                            exit(EXIT_FAILURE);
                        }
                        /* Check for communication termination (EOF) */
                        if (has_hit_eof()) {
                            done = 1;
                        } else { 
                            if ((msgLen = send(sockfd, buf, 
                              stdIN_len, 0)) == -1) {
                                perror("send");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                    if (fds[REMOTE].revents & POLLIN) {
                        if ((msgLen = recv(sockfd, buf, 
                           sizeof(buf), 0)) == -1) {
                            perror("recv");
                            exit(EXIT_FAILURE);
                        }
                        if (write_to_output(buf, msgLen) == -1) {
                            perror("write");
                            exit(EXIT_FAILURE);
                        }
                        /* Terminate communication (EOF) */
                        if (msgLen == 0) {
                            done = 1;
                        }
                    }
                } while ((!done) && (!exit_flag));

            /* or use terminal line */
            } else {
                fds[LOCAL].fd = STDIN_FILENO;
                fds[LOCAL].events = POLLIN;
                fds[LOCAL].revents = 0;
                fds[REMOTE] = fds[LOCAL];
                fds[REMOTE].fd = sockfd;    
 
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

    /* Handle Signals */
    sigAct.sa_handler = handler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SAFLAGS;
    
    if (sigaction(SIGINT, &sigAct, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
          
                do {
                    /* Poll stdin indefinitely */
                    if (poll(fds, (sizeof(fds)/sizeof(struct pollfd)),
                     -1) == -1) {
                        if (errno == EINTR) {
                            if ((msgLen = send(sockfd, 
                        "\nConnection closed. ^C to terminate.\n", 
                        strlen("\nConnection closed. ^C to"\
                        " terminate.\n"), 0)) == -1) {
                                perror("send");
                                exit(EXIT_FAILURE);
                            } 
                            break;
                        }
                        perror("poll");
                        exit(EXIT_FAILURE);
                    }
                    if (fds[LOCAL].revents & POLLIN) {
                        if ((stdIN_len = read(STDIN_FILENO, 
                           buf, MAXLEN)) == -1) {
                            perror("read");
                            exit(EXIT_FAILURE);
                        }
                        /* Check for communication termination (EOF) */
                        if (stdIN_len == 0) {
                            done = 1;
                        } else { 
                            if ((msgLen = send(sockfd, buf,
                             stdIN_len, 0)) == -1) {
                                perror("send");
                                exit(EXIT_FAILURE);
                            } 
                        }
                    }
                    if (fds[REMOTE].revents & POLLIN) {
                        if ((msgLen = recv(sockfd, buf,
                           sizeof(buf), 0)) == -1) {
                            perror("recv");
                            exit(EXIT_FAILURE);
                        }
                        if (write(STDOUT_FILENO, buf, msgLen) == -1) {
                            perror("write");
                            exit(EXIT_FAILURE);
                        }
                        /* Terminate Communication (EOF) */
                        if (msgLen == 0) {
                            done = 1;
                        }
                    }
                } while ((!done) && (!exit_flag));
            }    
        } else {
            printf("Client: %s declined connection\n", hostname);
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
            if (close(sockfd) == -1) {
                perror("close");
                exit(EXIT_FAILURE);
            }
            /* Terminate program */
            return 0;
        }
        
        /* Close ncurses window if applicable */ 
        if (!Nflag) {
            while ((exit_flag == 0) && (done == 1)) {
                pause();
            } 

            stop_windowing();
        } else {
            while ((exit_flag == 0) && (done == 1)) {
                pause();
            } 
        }
 
        /* CLose socket */
        if (vflag) {
            printf("\nClient: closing socket...");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
        if (close(sockfd) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        if (vflag) {
            printf("complete\n");
            if (fflush(stdout) == EOF) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
        }
    }


    return 0;
}
