#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

/* Constants */
#define NOT_FOUND 404
#define FOUND 200
#define MAX_CONNECTIONS 100
#define BUFFER_SIZE 1024
#define ERROR -1

/* Macros */
#define ARRAY_LENGTH(x) (sizeof x / sizeof *x)

/* Constants */
const char *found = "%s 200 OK\r\n";
const char *not_found = "%s 404\r\n";
const char *content_header = "Content-Type: %s\r\n";
const char *length_header = "Content-Length: %s\r\n\r\n";

/* HTTP request info */
typedef struct {
    char *method;
    char *URI;
    char *httpversion;
} http_request;

/* information that client needs */
typedef struct {
    int client;
    char *webroot;
} client_info;

/* Served file information, including mime types */
typedef struct {
    const char *extension;
    const char *mime_type;
} file_properties;

/* Hardcoded mime types */
const file_properties file_map[] = {
    {".html", "text/html"},
    {".jpg", "image/jpeg"},
    {".css", "text/css"},
    {".js", "text/javascript"}
};

#endif