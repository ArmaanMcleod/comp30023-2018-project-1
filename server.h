#ifndef SERVER_H
#define SERVER_H

/* Constants */
#define NOT_FOUND 404
#define FOUND 200
#define MAX_CONNECTIONS 100
#define BUFFER_SIZE 1024
#define ERROR -1

/* Macros */
#define ARRAY_LENGTH(x) (sizeof x / sizeof *x)

/* Header constants */
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