#ifndef SERVER_H
#define SERVER_H

/* Constants */
#define NOT_FOUND 404
#define FOUND 200
#define BACKLOG 100
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
extern const char *not_supported;
extern const char *no_content;

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

extern const file_properties file_map[];

/* Function prototypes */
int setup_listening_socket(int portno, int max_clients);

void parse_request(http_request *parameters, const char *response);

bool supported_file(const char *extension);

char *get_full_path(const char *webroot, const char *path, int *status);

void write_headers(int client, const char *data, const char *defaults);

size_t get_length_bytes(size_t bytes);

void write_content_length(int client, size_t bytes_read);

void read_write_file(int client, const char *path);

void construct_file_response(int client, const char *httpversion, 
                             const char *path, const char *status);

void process_client_request(int client);

#endif