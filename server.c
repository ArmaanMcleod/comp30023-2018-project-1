#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

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

/* Checks if a given extension is valid */
/* Verifies that it is either .js, .jpg, .css or .html */
bool supported_file(char *extension) {
    for (size_t i = 0; i < ARRAY_LENGTH(supported_extensions); i++) {
        if (strcmp(supported_extensions[i], extension) == 0) {
            return true;
        }
    }

    return false;
}

/* Checks if a file exists in the web directory */
char *check_file_exists(char *webroot, char *path, int *status) {
    char *full_path = NULL;
    *status = NOT_FOUND;

    /* Create an array big enough for the web root and path */
    full_path = malloc(strlen(webroot) + strlen(path) + 1);

    /* Combine web root and path */
    strcpy(full_path, webroot);
    strcat(full_path, path);

    /* Gets the extension after the first dot character */
    char *extension = strchr(full_path, '.');

    /* If nothing is found, return nothing*/
    if (extension == NULL) {
        return NULL;
    }
    
    /* If full path is accessible and file is supported, -
       return path and update status to 200 */
    if (access(full_path, F_OK) == 0 && supported_file(extension)) {
        *status = FOUND;
        return full_path;
    }

    /* If we get here, no path to a valid file is possible */
    return NULL;
}

void construct_file_response(int client, char *full_path) {
    return;
}

void process_client_request(int client, char *webroot) {
    char response[BUFFER_SIZE];
    http_request request;
    int n, status_code;

    /* Read in response */
    n = read(client, response, BUFFER_SIZE - 1);
    if (n == ERROR) {
        fprintf(stderr, "Error: cannot read request\n");
        exit(1);
    }

    /* Parse request parameters */
    parse_request(&request, response);

    /* get status of requested file */
    char *path = check_file_exists(webroot, request.URI, &status_code);

    if (status_code == FOUND) {

    }

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
           be accepted. Get back a new extension descriptor to communicate on. */
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