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

/* Header constants */
const char *found = "%s 200 OK\r\n";
const char *content_header = "Content-Type: %s\r\n";
const char *length_header = "Content-Length: %s\r\n\r\n";

const char *not_found = "%s 404 Not Found\r\n";
const char *not_supported = "Content-Type: application/octet-stream\r\n";
const char *no_content = "Content-Length: 0\r\n\r\n";

/* Hardcoded mime types */
const file_properties file_map[] = {
    {".html", "text/html"},
    {".jpg", "image/jpeg"},
    {".css", "text/css"},
    {".js", "text/javascript"}
};

/* Web root gloabl variable */
char *webroot = NULL;

/* Sets up listening socket for server */
static int setup_listening_socket(int portno, int max_clients) {
    struct sockaddr_in serv_addr;
    int sock, setopt = 1;

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
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
                &setopt, sizeof setopt) == ERROR) { 

        perror("Error: setting socket option for reusing address"); 
        exit(EXIT_FAILURE); 
    } 

    /* Bind address to the socket */
    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof serv_addr) == ERROR) {
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
static void exit_if_null() {
    if (ptr == NULL) {
        perror("Error: unexpected null pointer");
        exit(EXIT_FAILURE);
    }

    return;
}

/* Parses HTTP request header */
/* Gets method, URI and version */
/* Got inspiration to use strtok_r from Linux man page */
static void parse_request(http_request *parameters, const char *response) {
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
static bool supported_file(const char *extension) {
    for (size_t i = 0; i < ARRAY_LENGTH(file_map); i++) {

        /* If extension is the same here, return */
        if (strcmp(file_map[i].extension, extension) == 0) {
            return true;
        }
    }

    return false;
}

/* Gets full path of requested file */
static char *get_full_path(const char *webroot, 
                           const char *path, int *status) {

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
    if (extension != NULL && 
        access(full_path, F_OK) == 0 && 
        supported_file(extension)) {

        *status = FOUND;
    }

    /* Return the full path either way*/
    return full_path;
}

/* Write 200 response headers */
static void write_headers(int client, const char *data, const char *defaults) {
    char *buffer = malloc(strlen(data) + strlen(defaults) + 1);
    exit_if_null(buffer);

    /* Write into buffer */
    sprintf(buffer, defaults, data);

    if (write(client, buffer, strlen(buffer)) == ERROR) {
        perror("Error: cannot write to socket");
        exit(EXIT_FAILURE);
    }

    free(buffer);

    return;
}

/* Calculates length of number */
static size_t get_length_bytes(size_t bytes) {
    size_t temp = bytes, count = 0;

    while (temp != 0) {
        temp /= 10;
        count++;
    }

    return count;
}

/* Write content_length for requested file */
void write_content_length(int client, size_t bytes_read) {
    char *content_length = NULL;
    size_t length_bytes, total_bytes;

    /* Get number of digits in bytes read */
    length_bytes = get_length_bytes(bytes_read);
    total_bytes = strlen(length_header) + length_bytes;

    /* Write content length */
    content_length = malloc(total_bytes + 1);
    exit_if_null(content_length);

    /* copy bytes reead into buffer */
    snprintf(content_length, total_bytes + 1, "%zu", bytes_read);
    write_headers(client, content_length, length_header);

    /* Done with this buffer */
    free(content_length);

    return;
}

/* Write file requested from 200 response */
static void read_write_file(int client, const char *path) {
    FILE *requested_file = NULL;
    unsigned char *buffer = NULL;
    size_t  bytes_read, buffer_size;

    /* Open contents of file in binary mode*/
    requested_file = fopen(path, "rb");
    exit_if_null(requested_file);

    /* Get size of file */
    fseek(requested_file, 0, SEEK_END);
    long file_size = ftell(requested_file);
    fseek(requested_file, 0, SEEK_SET);

    /* Allocate buffer big enough to hold file */
    /* Since a file size can be anything, getting the file size beforehand -
       avoids the danger of buffer overflow */
    /* Having a sized buffer could be dangerous here */
    buffer_size = (size_t)file_size;
    buffer = malloc(buffer_size + 1);
    exit_if_null(buffer);

    /* Set everything to null terminating characters */
    /* Avoids having to null terminate the buffer later on */
    memset(buffer, '\0', buffer_size + 1);

    /* Write contents of file to client socket */
    bytes_read = fread(buffer, 1, file_size, requested_file);
    if (bytes_read == buffer_size) {

        /* Write content length header */
        write_content_length(client, bytes_read);
        
        /* Write body of header to socket */
        if (write(client, buffer, bytes_read) == ERROR) {
            perror("Error: cannot write to socket");
            exit(EXIT_FAILURE);
        }
    }

    /* buffer has served its purpose, free it up */
    free(buffer);

    fclose(requested_file);

    return;
}

static void construct_file_response(int client, const char *httpversion, 
                                    const char *path, const char *status) {

    char *requested_file_extension = NULL;
    bool found = false;

    /* Write the status header */
    write_headers(client, httpversion, status);

    /* Get the file extension */
    requested_file_extension = strrchr(path, '.');

    /* If no extension exists, write appropriate response and exit */
    if (requested_file_extension == NULL) {
        write(client, not_supported, strlen(not_supported));
        return;
    }

    /* otherwise, see if extension is served */
    for (size_t i = 0; i < ARRAY_LENGTH(file_map); i++) {
        if (strcmp(file_map[i].extension, requested_file_extension) == 0) {

            /* Write http content type */
            write_headers(client, file_map[i].mime_type, content_header);
            found = true;
            break;
        }
    }

    /* No extension was found, write appropriate response */
    if (!found) {
        write(client, not_supported, strlen(not_supported));
    }

    return;
}

static void process_client_request(int client) {
    char buffer[BUFFER_SIZE];
    char *path = NULL;
    http_request request;
    int status_code;

    /* Read in request */
    if (read(client, buffer, BUFFER_SIZE - 1) == ERROR) {
        perror("Error: cannot read request");
        exit(EXIT_FAILURE);
    }

    /* Parse request parameters */
    parse_request(&request, buffer);

    /* Get absolute path of requested file */
    /* Only needed for body of 200 response */
    path = get_full_path(webroot, request.URI, &status_code);

    /* Construct file responses, depending on status code */
    if (status_code == FOUND) {
        construct_file_response(client, request.httpversion, path, found);
        read_write_file(client, path);
    } else {
        construct_file_response(client, request.httpversion, path, not_found);
        write(client, no_content, strlen(no_content));
    }

    /* Free up all the pointers on the heap */
    free(request.method);
    free(request.URI);
    free(request.httpversion);

    free(path);

    /* Close the client socket */
    close(client);

    return;
}

/* Processes client request for a file */
static void *handle_client_request(void *args) {
    void *socket = NULL;
    int client;

    /* Extract threadpool contents */
    thread_pool *pool = args;

    while (true) {

        /* Critical section */
        pthread_mutex_lock(&pool->mutex);

        /* waiting for work to come up */
        while (queue_is_empty(pool->queue)) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        } 

        /* deque first task */
        socket = queue_dequeue(pool->queue);

        pthread_mutex_unlock(&pool->mutex);

        /* Get client socket id */
        client = *(int *)socket;

        /* process client task here */
        process_client_request(client);

    }

    pthread_exit(NULL);
}

/* Clean up the thread pool */
static void cleanup_pool(thread_pool *pool) {

    /* Join the threads back up together */
    for (size_t i = 0; i < MAX_THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    /* Free up the the queue */
    queue_free(pool->queue);

    /* Destroy the mutex and conditions */
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);

    /* Free up the thread pool */
    free(pool);

    return;
}

/* Create workers here */
static void create_workers(thread_pool *pool, size_t max_threads) {
    /* Create threadpool worker threads */
    for (size_t i = 0; i < max_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, 
                           handle_client_request, pool)) {

            perror("Error: cannot create thread");
            exit(EXIT_FAILURE);
        }
    }

    return;
}

/* Creates a new threadpool */
static thread_pool *initialise_threadpool() {
    thread_pool *pool = NULL;

    /* Create thread pool */
    pool = malloc(sizeof *pool);
    exit_if_null(pool);

    /* Initialise thread pool queue */
    pool->queue = queue_new();

    /* Initialise thread pool mutex */
    if (pthread_mutex_init(&pool->mutex, NULL)) {
        perror("Error: mutex init failed");
        exit(EXIT_FAILURE);
    }

    /* Initialise thread pool condition */
    if (pthread_cond_init(&pool->cond, NULL)) {
        perror("Error: cond init failed");
        exit(EXIT_FAILURE);
    }

    /* Create workers for thread pool */
    create_workers(pool, MAX_THREADS);

    return pool;
}

/* Add client work to task queue */
static void add_client_work(thread_pool *pool, int *client) {
    /* Critical section */
    pthread_mutex_lock(&pool->mutex);

    /* Add client to the queue */
    queue_enqueue(pool->queue, client);

    pthread_mutex_unlock(&pool->mutex);

    /* Send a signal to worker threads that a client task has been added */
    pthread_cond_signal(&pool->cond);
}

int main(int argc, char *argv[]) {
    int sockfd, client, portno;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof client_addr;
    thread_pool *pool = NULL;

    /* Check if enough command line arguements were given */
    if (argc != 3) {
        fprintf(stderr, "Usage: ./server [port number] [path to webroot]\n");
        exit(EXIT_FAILURE);
    }

    /* Convert port number to a digit */
    /* Assumes port number is valid */
    portno = atoi(argv[1]);

    /* Update global webroot */
    webroot = argv[2];

    pool = initialise_threadpool();

    /* Construct socket */
    sockfd = setup_listening_socket(portno, MAX_CONNECTIONS);

    /* loop that keeps fetching connections forever */
    /* allows server to be a persistent connection */
    while (true) {

        /* Accept a connection - block until a connection is ready to -
           be accepted. Fetch new extension descriptor to communicate on. */
        client = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (client == ERROR) {
            perror("Error: cannot open socket");
            continue;
        }

        add_client_work(pool, &client);
    }

    /* Close up the server socket, just in case */
    close(sockfd);

    /* Clean up the thread pool, just for good measure */
    cleanup_pool(pool);

    exit(EXIT_SUCCESS);
}