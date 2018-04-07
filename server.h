#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>

#include "queue.h"

/* Constants */
#define NOT_FOUND 404
#define FOUND 200
#define MAX_CONNECTIONS 100
#define MAX_THREADS 100
#define BUFFER_SIZE 1024
#define ERROR -1

/* Macros */
#define ARRAY_LENGTH(x) (sizeof x / sizeof *x)

/* webroot */
extern char *webroot;

/* Header constants */
extern const char *found;
extern const char *not_found;
extern const char *content_header;
extern const char *length_header;
extern const char *no_content;

/* HTTP request info */
typedef struct {
    char *method;
    char *URI;
    char *httpversion;
} http_request;

/* Thread pool information */
typedef struct {
    Queue *queue;
    pthread_t threads[MAX_THREADS];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}thread_pool;

/* Served file information, including mime types */
typedef struct {
    const char *extension;
    const char *mime_type;
} file_properties;

extern const file_properties file_map[];

#endif