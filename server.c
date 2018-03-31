#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"

/* Sets up listening socket for server */
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
    printf("Listening on port: %d.\n", portno);

    /* Listen on socket - means we're ready to accept connections - 
       incoming connection requests will be queued */
    if (listen(sock, max_clients) == ERROR) {
        fprintf(stderr, "Error: cannot listen on socket\n");
        exit(1);
    }
    printf("Listening for incoming connections...\n");

    return sock;
}

/* Parses HTTP request header */
/* Gets method, URI and version */
void parse_request(http_request *parameters, char *response) {
    /* Initially fill request parameters in an array */
    char *request[NUM_PARAMS]; 

    /* extract everything up until new line */
    char *temp = response;
    strtok(temp, "\n");

    /* If line was found */
    if (temp != NULL) {

        /* Split line by spaces and add it to array */
        size_t count = 0;
        char *split_string = strtok(temp, " ");
        while (split_string != NULL) {

            /* Make sure we dont go beyond the array */
            if (count < NUM_PARAMS) {
                request[count] = strdup(split_string);
                count++;
            }
            split_string = strtok(NULL, " ");
        }
    }

    /* Set paramateters to struct object */
    /* Easier to handle than an array */
    parameters->method = strdup(request[0]);
    parameters->URI = strdup(request[1]);
    parameters->httpversion = strdup(request[2]);
}

int check_file(char *webroot, char *path) {
    char *full_path;

    full_path = malloc(strlen(webroot) + strlen(path) + 1);
    if (full_path == NULL) {
        fprintf(stderr, "Error: annot allocate string\n");
    }

    strcpy(full_path, webroot);
    strcat(full_path, path);

    if (access(full_path, F_OK) == 0) {
        return FOUND;
    } 

    return NOT_FOUND;

}

void process_client_request(int client, char *webroot) {
    char response[BUFFER_SIZE];
    http_request request;
    int n;

    /* Read in response */
    n = read(client, response, BUFFER_SIZE - 1);
    if (n == ERROR) {
        fprintf(stderr, "Error: cannot read request\n");
        exit(1);
    }

    /* Parse request parameters */
    parse_request(&request, response);

    int file_check = check_file(webroot, request.URI);

    printf("%d\n", x);

    //printf("%s\n", request.method);
    //printf("%s\n", request.URI);
    //printf("%s\n", request.httpversion);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof client_addr;
    int portno;

    /* Check if enough command line arguements were given */
    if (argc != 3) {
        fprintf(stderr, "Usage: ./server [port number] [path to webroot]\n");
        exit(1);
    }

    /* Convert port number to a digit */
    /* Assumes port number is valid */
    portno = atoi(argv[1]);

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

        /* Process incoming request */
        process_client_request(newsockfd, argv[2]);

    }

    close(sockfd);

    return 0;
}