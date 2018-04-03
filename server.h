#ifndef SERVER_H
#define SERVER_H

/* Response status */
#define NOT_FOUND 404
#define FOUND 200

/* buffer sizes */
#define MAX_THREADS 100
#define MAX_CLIENTS 1024
#define BUFFER_SIZE 1024

/* Error flag */
#define ERROR -1

#define ARRAY_LENGTH(x) (sizeof x / sizeof *x)

typedef struct {
    char *method;
    char *URI;
    char *httpversion;
} http_request;

/* Mime types */
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