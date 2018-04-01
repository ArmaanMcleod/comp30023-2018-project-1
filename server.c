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
        exit(EXIT_FAILURE);
    }
    printf("Listening socket created.\n");

    memset(&serv_addr, '\0', sizeof serv_addr);

    /* Create address we're going to listen on (given port number) -
       converted to network byte order & any IP address for -
       this machine */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Set socket option SO_REUSEADDR. If a recently closed server wants to -
       use this port, and some of the leftover chunks is lingering around -
       we can still use this port */
    int yes=1; 
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == ERROR) { 
        fprintf(stderr, "Error setting socket option for reusing address\n"); 
        exit(EXIT_FAILURE); 
    }  

    /* Bind address to the socket */
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof serv_addr) == ERROR) {
        fprintf(stderr, "Error: cannot bind address to socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Binding done.\n");
    printf("Listening on port: %d.\n", portno);

    /* Listen on socket - means we're ready to accept connections - 
       incoming connection requests will be queued */
    if (listen(sock, max_clients) == ERROR) {
        fprintf(stderr, "Error: cannot listen on socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Listening for incoming connections...\n");

    return sock;
}

/* Parses HTTP request header */
/* Gets method, URI and version */
void parse_request(http_request *parameters, char *response) {
    char *saveptr = NULL, *path = NULL;

    /* Copy over the response */
    char *copy = strdup(response);

    /* Extract just the first line */
    path = strtok_r(copy, "\n", &saveptr);

    /* Extract the method */
    path = strtok_r(copy, " ", &saveptr);
    parameters->method = strdup(path);

    /* Extract the URI */
    path = strtok_r(NULL, " ", &saveptr);
    parameters->URI = strdup(path);

    /* Extract the http version */
    parameters->httpversion = strdup(saveptr);
    parameters->httpversion[strlen(saveptr)-1] = '\0';

    free(copy);

}

/* Checks if a given extension is valid */
/* Verifies that it is either .js, .jpg, .css or .html */
bool supported_file(char *extension) {
    for (size_t i = 0; i < ARRAY_LENGTH(supported_extensions); i++) {

        /* If extension is the same here, return */
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
    if (full_path == NULL) {
        fprintf(stderr, "Error: cannot allocate space for path\n");
        exit(1);
    }

    /* Combine web root and path */
    strcpy(full_path, webroot);
    strcat(full_path, path);

    /* Gets the extension after the first dot character */
    char *extension = strchr(full_path, '.');

    /* If nothing is found, return no path*/
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

/* Write file requested from 200 response */
void read_write_file(int client, FILE *requested_file) {
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read = 0;

    /* Write contents of file to client socket */
    while ((bytes_read = fread(buffer, 1, sizeof buffer, requested_file)) > 0) {
        write(client, buffer, bytes_read);
    }
}

/* Write 200 response headers */
void write_headers(int client, const char *data, const char *defaults) {
    char *buffer = malloc(strlen(data) + strlen(defaults) + 1);
    sprintf(buffer, defaults, data);
    write(client, buffer, strlen(buffer));
    free(buffer);
}

/* Construct a file response for a 200 header */
void construct_file_response(int client, char *httpversion, char *full_path) {
    char *request_file_extension = NULL;
    FILE *requested_file = NULL;

    /* boilerplate headers */
    const char *status_header = "%s 200 OK\r\n";
    const char *content_header = "Content-Type: %s\r\n\r\n";

    /* Get the file extension */
    request_file_extension = strchr(full_path, '.');

    /* First write the header http status */
    write_headers(client, httpversion, status_header);

    /* Go over the mime types and match the correct one */
    for (size_t i = 0; i < ARRAY_LENGTH(mime_map); i++) {
        if (strcmp(mime_map[i].extension, request_file_extension) == 0) {

            /* Write http content type */
            write_headers(client, mime_map[i].mime_type, content_header);

            break;
        }
    }

    /* Open contents of file in binary mode*/
    requested_file = fopen(full_path, "rb");
    if (requested_file == NULL) {
        fprintf(stderr, "Error: cannot open file\n");
        exit(1);
    }

    read_write_file(client, requested_file);

    fclose(requested_file);
}

/* Processes client request for a file */
void process_client_request(int client, char *webroot) {
    char copy[BUFFER_SIZE];
    http_request request;
    int n, status_code;

    /* Read in copy */
    n = read(client, copy, BUFFER_SIZE - 1);
    if (n == ERROR) {
        fprintf(stderr, "Error: cannot read request\n");
        exit(1);
    }

    /* Parse request parameters */
    parse_request(&request, copy);

    /* get path of requested file */
    /* only needed for 200 response */
    char *path = check_file_exists(webroot, request.URI, &status_code);

    if (status_code == FOUND) {
        construct_file_response(client, request.httpversion, path);
    }

    free(request.method);
    free(request.URI);
    free(request.httpversion);

    free(path);
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

        /* We can now close this client socket */
        close(newsockfd);
    }

    close(sockfd);

    return 0;
}