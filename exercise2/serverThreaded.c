/* A threaded server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFERLENGTH 256

/* displays error messages from system calls */
void error(char *msg) {
    perror(msg);
    exit(1);
}


int lineNo = 0;
FILE *fp;
char *fileName;

pthread_mutex_t mut; /* the lock */

typedef void(*split_fn)(const char *, size_t, void *);

void split(const char *str, char sep, split_fn fun, void *data) {
    unsigned int start = 0, stop;
    for (stop = 0; str[stop]; stop++) {
        if (str[stop] == sep) {
            fun(str + start, stop - start, data);
            start = stop + 1;
        }
    }
    fun(str + start, stop - start, data);
}

/* the procedure called for each request */
void *processRequest(void *args) {
    int *newsockfd = (int *) args;
    char buffer[BUFFERLENGTH];
    int n;
    while (1) {
        bzero (buffer, BUFFERLENGTH);
        n = read(*newsockfd, buffer, BUFFERLENGTH - 1);
//        printf("Here is the message: `%s` of length: %lu\n", buffer, strlen(buffer));
        if (n < 0) {
            printf("ERROR reading from socket");
            break;
        }

        char *token;
        if (buffer[0] == '\n') {
            char out[BUFFERLENGTH];
            pthread_mutex_lock(&mut); /* lock exclusive access to variable isExecuted */
            sprintf(out, "%d %s", lineNo++, "\n");
            fp = fopen(fileName, "a");
            fputs(out, fp);
            fclose(fp);
            pthread_mutex_unlock(&mut); /* release the lock */
        }

        /* get the first token */
        token = strtok(buffer, "\n");

        /* walk through other tokens */
        while (token != NULL) {
            printf("%s\n", token);
            char out[BUFFERLENGTH];
            pthread_mutex_lock(&mut); /* lock exclusive access to variable isExecuted */
            sprintf(out, "%d %s\n", lineNo++, token);
            fp = fopen(fileName, "a");
            fputs(out, fp);
            fclose(fp);
            pthread_mutex_unlock(&mut); /* release the lock */
            token = strtok(NULL, "\n");
        }


//        if (buffer[0] == '\n' && strlen(buffer) > 1) {
//            char out[BUFFERLENGTH];
//            sprintf(out, "\n%d ^%s$\n", lineNo++, "\n");
//            char out2[BUFFERLENGTH];
//            char tempBuffer[BUFFERLENGTH];
//            for (unsigned int i = 0; i < strlen(buffer) - 2; ++i) {
//                tempBuffer[i] = buffer[i + 1];
//            }
//            sprintf(out2, "\n%d ^%s$\n", lineNo++, tempBuffer);
//            pthread_mutex_lock(&mut); /* lock exclusive access to variable isExecuted */
//            fp = fopen(fileName, "a");
//            fputs(out, fp);
//            fputs(out2,fp);
//            fclose(fp);
//            pthread_mutex_unlock(&mut); /* release the lock */
//        }else{
//            char out[BUFFERLENGTH];
//
//            sprintf(out, "\n%d %s\n", lineNo++, buffer);
//            pthread_mutex_lock(&mut); /* lock exclusive access to variable isExecuted */
//            fp = fopen(fileName, "a");
//            fputs(out, fp);
//            fclose(fp);
//            pthread_mutex_unlock(&mut); /* release the lock */
//        }
        /* send the reply back */
        n = write(*newsockfd, buffer, BUFFERLENGTH);
        if (n < 0) {
            printf("ERROR writing to socket\n");
            break;
        }

    }
    printf("Cleaning up\n");
    close(*newsockfd); /* important to avoid memory leak */
    free(newsockfd);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    socklen_t clilen;
    int sockfd, portno;
    char buffer[BUFFERLENGTH];
    struct sockaddr_in6 serv_addr, cli_addr;
    int result;


    if (argc != 3) {
        fprintf(stderr, "usage: %s <port> <log file>\n", argv[0]);
        exit(1);
    }

    /* check port number */
    portno = atoi(argv[1]);
    if ((portno < 0) || (portno > 65535)) {
        fprintf(stderr, "%s: Illegal port number, exiting!\n", argv[0]);
        exit(1);
    }
    fileName = argv[2];
    fp = fopen(fileName, "w");
    fclose(fp);

    /* create socket */
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero ((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons (portno);

    /* bind it */
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* ready to accept connections */
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    /* now wait in an endless loop for connections and process them */
    while (1) {

        pthread_t server_thread;

        int *newsockfd; /* allocate memory for each instance to avoid race condition */
        pthread_attr_t pthread_attr; /* attributes for newly created thread */

        newsockfd = malloc(sizeof(int));
        if (!newsockfd) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
        }

        /* waiting for connections */
        *newsockfd = accept(sockfd,
                            (struct sockaddr *) &cli_addr,
                            &clilen);
        if (*newsockfd < 0)
            error("ERROR on accept");
        bzero (buffer, BUFFERLENGTH);

        /* create separate thread for processing */
        if (pthread_attr_init(&pthread_attr)) {
            fprintf(stderr, "Creating initial thread attributes failed!\n");
            exit(1);
        }

        if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED)) {
            fprintf(stderr, "setting thread attributes failed!\n");
            exit(1);
        }
        result = pthread_create(&server_thread, &pthread_attr, processRequest, (void *) newsockfd);
        if (result != 0) {
            fprintf(stderr, "Thread creation failed!\n");
            exit(1);
        }


    }
    return 0;
}
