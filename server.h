#ifndef SERVER_H
#define SERVER_H

/* Response status */
#define NOT_FOUND 404
#define FOUND 200

#define MAX_CLIENTS 1024
#define BUFFER_SIZE 256
#define MAX_PATH_LENGTH 512
#define NUM_PARAMS 3

/* Error flag */
#define ERROR -1

#define ARRAY_LENGTH(x) (sizeof x / sizeof *x)

const char *supported_extensions[] = {".html", ".jpg", ".css", ".js"};

typedef struct {
    char *method;
    char *URI;
    char *httpversion;
} http_request;

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