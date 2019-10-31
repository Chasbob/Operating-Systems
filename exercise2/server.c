/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>

#define BUFFERLENGTH 256

/* displays error messages from system calls */
void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    char *logFile;
    socklen_t clilen;
    int sockfd, newsockfd, portno;
    char buffer[BUFFERLENGTH];
    struct sockaddr_in6 serv_addr, cli_addr;
    int n;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    logFile = argv[2];
    /* check port number */
    portno = atoi(argv[1]);
    if ((portno < 0) || (portno > 65535)) {
        fprintf(stderr, "%s: Illegal port number, exiting!\n", argv[0]);
        exit(1);
    }
    FILE *fp;
    printf("Opening file %s\n", logFile);
    fp = fopen(logFile, "a");

    /* create socket */
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    // bzero((char *)&serv_addr, sizeof(serv_addr));
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(portno);

    /* bind it */
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* ready to accept connections */
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    /* now wait in an endless loop for connections and process them */
    while (1) {

        /* waiting for connections */
        newsockfd = accept(sockfd,
                           (struct sockaddr *) &cli_addr,
                           &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        // bzero(buffer, BUFFERLENGTH);
        memset(buffer, 0, BUFFERLENGTH);

        /* read the data */
        n = read(newsockfd, buffer, BUFFERLENGTH - 1);
        if (n < 0)
            error("ERROR reading from socket");
        // fprintf(fp, "This is line %s\n", buffer);
        if (strcmp(buffer, "exit\n") == 0) {
            printf("Closing file.");
            fclose(fp);
            break;
        } else {
            fp = fopen(argv[2], "a+");
            fputs(buffer, fp);
            fclose(fp);

            // printf("Here is the message: %s", buffer);
        }

        /* send the reply back */
        n = write(newsockfd, "I got your message", 18);
        if (n < 0)
            error("ERROR writing to socket");

        close(newsockfd); /* important to avoid memory leak */
    }
    return 0;
}
