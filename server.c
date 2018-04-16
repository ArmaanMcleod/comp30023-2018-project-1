/* COMP30023 Computer Systems - Semester 1 2018
 * Assignment 1 - HTTP multi-threaded Web server
 * File: server.c
 * Purpose: main server program. Server code logic implemented here
 * Author: Armaan Dhaliwal-McLeod
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>

/* Helper header files included */
#include "threadpool.h"
#include "http.h"

/* size variables for listening queue and buffers */
#define BACKLOG 100
#define BUFFER_SIZE 1024

const char *html = "404 Not Found";

/* Web root global variable */
char *webroot = NULL;

/* signal flag for when server is closed */
volatile sig_atomic_t running = false;

/* signal handling function */
/* Updates running variables when SIGINT, SIGTERM or SIGUIT is triggered */
static void signal_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM || signum == SIGQUIT) {
        fprintf(stderr, "Server ended\n");
        running = true;
    }
    return;
}

/* Sets up listening socket for server */
static int setup_listening_socket(int portno, int max_clients) {
    struct sockaddr_in serv_addr;
    int sock, reuse = 1;

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
                   &reuse, sizeof reuse) == ERROR) {

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

/* Process client request */
/* Function which gets dispatched to worker threads */
static void process_client_request(int client) {
    char buffer[BUFFER_SIZE] = {0};
    char *path = NULL;
    http_request_t request;
    int status_code;

    /* Read in request from client socket */
    if (read(client, buffer, BUFFER_SIZE - 1) == ERROR) {
        perror("Error: cannot read request");
        exit(EXIT_FAILURE);
    }

    /* Parse request parameters */
    parse_request(&request, buffer);

    /* Get absolute path of requested file */
    /* Only needed for body of 200 response */
    path = get_full_path(request.URI, webroot, &status_code);

    /* Construct file responses, depending on status code */
    if (status_code == FOUND) {
        construct_file_response(client, request.httpversion, path, found);
        read_write_file(client, path);
    } else {
        construct_file_response(client, request.httpversion, path, not_found);
        write(client, no_content, strlen(no_content));
        write(client, html, strlen(html));
    }

    /* Free up all the pointers allocated */
    free(request.method);
    free(request.URI);
    free(request.httpversion);

    free(path);

    /* Close the client socket */
    close(client);

    return;
}

int main(int argc, char *argv[]) {
    int sockfd, client, portno;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof client_addr;
    thread_pool *pool = NULL;
    struct sigaction action;

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

    pool = initialise_threadpool(process_client_request);

    /* Construct socket */
    sockfd = setup_listening_socket(portno, BACKLOG);

    /* Setup signal handler */
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    /* Check if ctrl-C, ctrl-\ or kill signal is triggered */
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);

    /* loop that keeps fetching connections forever until server dies */
    while (!running) {

        /* Accept a connection - block until a connection is r./eady to -
           be accepted. Fetch new extension descriptor to communicate on. */
        client = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (client == ERROR && errno == EINTR) {
            perror("Connection closed");
            break;
        }

        add_client_work(pool, &client);

    }

    /* Close up the server socket, just in case */
    close(sockfd);

    /* Clean up thread pool */
    /* I'm a good citizen that wants no memory leaks */
    cleanup_pool(pool);

    exit(EXIT_SUCCESS);
}
