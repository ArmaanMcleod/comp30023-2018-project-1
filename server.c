#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"

int setup_listening_socket(int portno, int max_clients) {
    struct sockaddr_in serv_addr;
    int sock;

     /* Setup TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == ERROR) {
        fprintf(stderr, "Error: cannot open socket");
        exit(1);
    }
    printf("Listening socket created.\n");

    bzero(&serv_addr, sizeof serv_addr);

    /* Create address we're going to listen on (given port number) -
       converted to network byte order & any IP address for -
       this machine */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Bind address to the socket */
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof serv_addr) == ERROR) {
        fprintf(stderr, "Error: cannot bind address to socket\n");
        exit(1);
    }
    printf("Binding done.\n");

    /* Listen on socket - means we're ready to accept connections - 
       incoming connection requests will be queued */
    if (listen(sock, max_clients) == ERROR) {
        fprintf(stderr, "Error: cannot listen on socket\n");
        exit(1);
    }
    printf("Listening for incoming connections...\n");

    return sock;
}

void parse_http_request(http_request *parameters, char buffer[]) {
    char *request[NUM_PARAMS]; 

    char *temp = buffer;
    strtok(temp, "\n");

    if (temp != NULL) {

        size_t count = 0;
        char *split_string = strtok(temp, " ");
        while (split_string != NULL) {
            if (count < NUM_PARAMS) {
                request[count] = split_string;
                count++;
            }
            split_string = strtok(NULL, " ");
        }
    }

    parameters->method = request[0];
    parameters->URI = request[1];
    parameters->httpversion = request[2];
}


int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof client_addr;
    char buffer[BUFFER_SIZE];
    int portno, n;

    /* Check if enough command line arguements were given */
    if (argc != 3) {
        fprintf(stderr, "Usage: ./server [port number] [path to webroot]\n");
        exit(1);
    }

    /* Convert port number to a digit */
    /* Assumes port number is valid */
    char *endptr = NULL;
    portno = strtoul(argv[1], &endptr, BASE);
    if (portno == 0 || *endptr != '\0') {
        fprintf(stderr, "Error: port number is not valid\n");
        exit(1);
    }

    /* Construct socket */
    sockfd = setup_listening_socket(portno, MAX_CLIENTS);

    /* loop that keeps fetching connections forever */
    while (1) {

        /* Accept a connection - block until a connection is ready to -
           be accepted. Get back a new file descriptor to communicate on. */
        newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (newsockfd == ERROR) {
            fprintf(stderr, "Error: cannot open socket\n");
            continue;
        }

        bzero(buffer, BUFFER_SIZE);

        n = read(newsockfd, buffer, BUFFER_SIZE - 1);
        if (n == ERROR) {
            fprintf(stderr, "Error: cannot read request\n");
            continue;
        }

        http_request parameters;

        parse_http_request(&parameters, buffer);

        printf("%s\n", parameters.method);
        printf("%s\n", parameters.URI);
        printf("%s\n", parameters.httpversion);

    }

    close(sockfd);

    return 0;
}