#ifndef SERVER_H
#define SERVER_H

/* Response status */
#define NOT_FOUND 404
#define FOUND 200

#define MAX_CLIENTS 1024
#define BUFFER_SIZE 256
#define BASE 10

/* Error flag */
#define ERROR -1

/* Content of http request */
typedef struct {
    char *method;
    char *URI;
    char *http_version;
} http_request;

/* Content of http response header */
typedef struct {
    char *status;
    char *content_type;
} response_header;

/* Mime types */
typedef struct {
    const char *extension;
    const char *mime_type;
} mime_types;

/* Hardcoded mime types */
const mime_types map[] = {
    {".html", "text/html"},
    {".jpg", "image/jpeg"},
    {".css", "text/css"},
    {".js", "application/javascript"}
};

#endif