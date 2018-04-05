#include "server.h"
#include "queue.h"

char *webroot = NULL;

/* Sets up listening socket for server */
int setup_listening_socket(int portno, int max_clients) {
    struct sockaddr_in serv_addr;
    int sock, on = 1;

     /* Setup TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == ERROR) {
        perror("Error: cannot open socket");
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
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) == ERROR) { 
        perror("Error: setting socket option for reusing address"); 
        exit(EXIT_FAILURE); 
    } 

    /* Bind address to the socket */
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof serv_addr) == ERROR) {
        perror("Error: cannot bind address to socket");
        exit(EXIT_FAILURE);
    }

    printf("Binding done.\n");
    printf("Listening on port: %d.\n", portno);

    /* Listen on socket - means we're ready to accept connections - 
       incoming connection requests will be queued */
    if (listen(sock, max_clients) == ERROR) {
        perror("Error: cannot listen on socket");
        exit(EXIT_FAILURE);
    }
    printf("Listening for incoming connections...\n");

    return sock;
}

/* Used for checking null pointers */
void exit_if_null(void *ptr) {
    if (ptr == NULL) {
        perror("Error: unexpected null pointer");
        exit(EXIT_FAILURE);
    }
}

/* Parses HTTP request header */
/* Gets method, URI and version */
void parse_request(http_request *parameters, char *response) {
    char *saveptr = NULL, *path = NULL, *copy = NULL;

    /* Copy over the response */
    copy = strdup(response);
    exit_if_null(copy);

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
    printf("%s\n", extension);
    for (size_t i = 0; i < ARRAY_LENGTH(file_map); i++) {

        /* If extension is the same here, return */
        if (strcmp(file_map[i].extension, extension) == 0) {
            return true;
        }
    }

    return false;
}

/* Gets full path of requested file */
char *get_full_path(const char *webroot, const char *path, int *status) {
    char *full_path = NULL, *extension = NULL;
    *status = NOT_FOUND;

    /* Create an array big enough for the web root and path */
    full_path = malloc(strlen(webroot) + strlen(path) + 1);
    exit_if_null(full_path);

    /* Combine web root and path */
    strcpy(full_path, webroot);
    strcat(full_path, path);

    /* Gets the extension after the first dot character */
    extension = strrchr(full_path, '.');

    /* If full path is accessible and file is supported, update status to 200 */
    if (extension != NULL && access(full_path, F_OK) == 0 && supported_file(extension)) {
        *status = FOUND;
    }

    /* Return the full path either way*/
    return full_path;
}

/* Write 200 response headers */
void write_headers(int client, const char *data, const char *defaults) {
    char *buffer = malloc(strlen(data) + strlen(defaults) + 1);
    exit_if_null(buffer);

    /* Write into buffer */
    sprintf(buffer, defaults, data);

    if (write(client, buffer, strlen(buffer)) == ERROR) {
        perror("Error: cannot write to socket");
        exit(EXIT_FAILURE);
    }

    free(buffer);
}

/* Calculates length of number */
size_t get_length_bytes(size_t bytes) {
    size_t temp = bytes, count = 0;

    while (temp != 0) {
        temp /= 10;
        count++;
    }

    return count;
}

/* Write file requested from 200 response */
void read_write_file(int client, const char *path) {
    FILE *requested_file = NULL;
    char *content_length = NULL;
    size_t length_bytes, total_bytes, bytes_read;

    /* Open contents of file in binary mode*/
    requested_file = fopen(path, "rb");
    exit_if_null(requested_file);

    fseek(requested_file, 0, SEEK_END);
    long file_size = ftell(requested_file);
    fseek(requested_file, 0, SEEK_SET);

    buffer_size = (size_t)file_size + 1;
    char *buffer = malloc(buffer_size);
    exit_if_null(buffer);

    memset(buffer, '\0', buffer_size);

    /* Write contents of file to client socket */
    bytes_read = fread(buffer, 1, file_size, requested_file);
    if (bytes_read == file_size) {
        /* Get number of digits in bytes read */
        length_bytes = get_length_bytes(bytes_read);
        total_bytes = strlen(length_header) + length_bytes;

        /* Write content length */
        content_length = malloc(total_bytes + 1);
        exit_if_null(content_length);

        snprintf(content_length, total_bytes + 1, "%zu", bytes_read);
        write_headers(client, content_length, length_header);
        
        /* Write body of header to socket */
        if (write(client, buffer, bytes_read) == ERROR) {
            perror("Error: cannot write to socket");
            exit(EXIT_FAILURE);
        }

        free(content_length);
    }

    free(buffer);

    fclose(requested_file);
}

void construct_file_response(int client, const char *httpversion, const char *path, const char *status) {
    char *requested_file_extension = NULL;

    /* Write the status header */
    write_headers(client, httpversion, status);

    /* Get the file extension */
    requested_file_extension = strrchr(path, '.');

    /* Need to make sure an extension exists first */
    if (requested_file_extension != NULL) {
        for (size_t i = 0; i < ARRAY_LENGTH(file_map); i++) {
            if (strcmp(file_map[i].extension, requested_file_extension) == 0) {

                /* Write http content type */
                write_headers(client, file_map[i].mime_type, content_header);

                break;
            }
        }
    }
}

/* Processes client request for a file */
void *process_client_request(void *args) {
    char buffer[BUFFER_SIZE];
    char *path = NULL;
    http_request request;
    int status_code, client;

    client = *(int *)args;

    /* Read in request */
    if (read(client, buffer, BUFFER_SIZE - 1) == ERROR) {
        perror("Error: cannot read request");
        exit(EXIT_FAILURE);
    }

    /* Parse request parameters */
    parse_request(&request, buffer);

    /* get path of requested file */
    /* only needed for 200 response */
    path = get_full_path(webroot, request.URI, &status_code);

    if (status_code == FOUND) {
        construct_file_response(client, request.httpversion, path, found);
        read_write_file(client, path);
    } else {
        construct_file_response(client, request.httpversion, path, not_found);
    }

    free(request.method);
    free(request.URI);
    free(request.httpversion);

    free(path);

    close(client);

    return NULL;

}

int main(int argc, char *argv[]) {
    int sockfd, client, portno;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof client_addr;
    pthread_t thread_id;

    /* Check if enough command line arguements were given */
    if (argc != 3) {
        fprintf(stderr, "Usage: ./server [port number] [path to webroot]\n");
        exit(EXIT_FAILURE);
    }

    /* Convert port number to a digit */
    /* Assumes port number is valid */
    portno = atoi(argv[1]);

    webroot = argv[2];

    /* Construct socket */
    sockfd = setup_listening_socket(portno, MAX_CONNECTIONS);

    /* loop that keeps fetching connections forever */
    /* allows server to be a persistent connection */
    while (true) {

        /* Accept a connection - block until a connection is ready to -
           be accepted. Get back a new extension descriptor to communicate on. */
        client = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (client == ERROR) {
            perror("Error: cannot open socket");
            continue;
        }

        /* Create new thread */
        if (pthread_create(&thread_id, NULL, process_client_request, &client)) {
            perror("Error: failed to create thread");
            exit(EXIT_FAILURE);
        }

        /* Detach this thread */
        if (pthread_detach(thread_id)) {
            perror("Error: failed to detach thread");
            exit(EXIT_FAILURE);
        }

    }
    close(sockfd);
}