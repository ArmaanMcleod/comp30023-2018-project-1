/* COMP30023 Computer Systems - Semester 1 2018
 * Assignment 1 - HTTP multi-threaded Web server
 * Author: Armaan Dhaliwal-McLeod
 * File: http.h
 * Purpose: http handling header file. Defines contents and defintions for
            handling http requests
 */

#ifndef HTTP_H
#define HTTP_H

/* Status code flags */
#define NOT_FOUND 404
#define FOUND 200
#define ERROR -1

/* Array length macro for calculating length */
#define ARRAY_LENGTH(x) (sizeof x / sizeof *x)

/* Header constants, used as boilerplates for http responses */
extern const char found[];
extern const char not_found[];
extern const char content_header[];
extern const char length_header[];
extern const char not_supported[];
extern const char no_content[];

/* HTTP request information struct */
typedef struct {
    char *method;
    char *URI;
    char *httpversion;
} http_request_t;

/* Served file properties, including extension and mime type */
typedef struct {
    const char *extension;
    const char *mime_type;
} file_properties_t;

extern const file_properties_t file_map[];

/* Function prototypes */
void parse_request(http_request_t *parameters, const char *response);
char *get_full_path(const char *path, const char *webroot, int *status);
void read_write_file(int client, const char *path);
void construct_file_response(int client, const char *httpversion,
                             const char *path, const char *status);

#endif
