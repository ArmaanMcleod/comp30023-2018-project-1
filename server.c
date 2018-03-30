#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#define BUFFERSIZE 256
#define MAX_PORTS 65535
#define BASE 10

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, n;
    char buffer[BUFFERSIZE];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    unsigned long portno;
    char *endptr = NULL;

    /* Ensure port is provided */
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }

    /* Create TCP socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    /* Erase data */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    /* Convert port number to integer */
    portno = strtoul(argv[1], &endptr, BASE);

    /* Check if port number is a valid digit */
    if (*endptr != '\0') {
        fprintf(stderr, "ERROR, portno is not an integer\n");
        exit(EXIT_FAILURE);
    }

    if (portno < 0 || portno > MAX_PORTS) {
        fprintf(stderr, "ERROR, portno out of range\n");
        exit(EXIT_FAILURE);
    }

    

    exit(EXIT_SUCCESS);
}