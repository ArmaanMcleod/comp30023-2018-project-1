#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"

int setup_socket(int portno, int max_clients) {
    struct sockaddr_in serv_addr;
    int sock;

    /* Create address we're going to listen on (given port number) -
       converted to network byte order & any IP address for -
       this machine */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    bzero((char *) &serv_addr, sizeof(serv_addr));

    /* Setup TCP socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == ERROR) {
        fprintf(stderr, "Error: cannot open socket");
        exit(1);
    }

    /* Bind address to the socket */
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof serv_addr) == ERROR) {
        fprintf(stderr, "Error: cannot bind address to socket\n");
        exit(1);
    }

    /* Listen on socket - means we're ready to accept connections - 
       incoming connection requests will be queued */
    if (listen(sock, max_clients) == ERROR) {
        fprintf(stderr, "Error: cannot listen on socket\n");
        exit(1);
    }

    return sock;
}

int main(int argc, char *argv[]) {
    int socket, cli, pid;
    struct sockaddr_in client;
    socklen_t clilen;
    int portno;

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
        fprintf(stderr, "Error: port number is not a digit\n");
        exit(1);
    }

    /* Construct socket */
    socket = setup_socket(portno, MAX_CLIENTS);




    return 0;
}