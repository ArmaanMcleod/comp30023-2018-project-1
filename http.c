/* COMP30023 Computer Systems - Semester 1 2018
 * Assignment 1 - HTTP multi-threaded Web server
 * Author: Armaan Dhaliwal-McLeod
 * File: http.c
 * Purpose: http handling module. Implements http handling of requests and
            responses
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdbool.h>
 #include <unistd.h>

 #include "http.h"

 /* Header boilterplate strings */
const char *found = "%s 200 OK\r\n";
const char *content_header = "Content-Type: %s\r\n";
const char *length_header = "Content-Length: %s\r\n\r\n";

const char *not_found = "%s 404 Not Found\r\n";
const char *not_supported = "Content-Type: application/octet-stream\r\n";
const char *no_content = "Content-Length: 0\r\n\r\n";

/* Hardcoded mime types */
const file_properties_t file_map[] = {
    {".html", "text/html"},
    {".jpg", "image/jpeg"},
    {".css", "text/css"},
    {".js", "text/javascript"}
};

 /* Parses HTTP request header */
 /* Gets method, URI and version and inserts them in struct */
 void parse_request(http_request_t *parameters, const char *response) {
     char *saveptr = NULL, *path = NULL, *copy = NULL;

     /* Copy over the response */
     copy = strdup(response);
     if (!copy) {
         perror("Error: strdup() failed to copy response");
         exit(EXIT_FAILURE);
     }

     /* Extract just the first line */
     path = strtok_r(copy, "\n", &saveptr);

     /* Extract the method */
     path = strtok_r(copy, " ", &saveptr);

     parameters->method = strdup(path);
     if (!parameters->method) {
         perror("Error: strdup() failed to copy path");
         exit(EXIT_FAILURE);
     }

     /* Extract the URI */
     path = strtok_r(NULL, " ", &saveptr);

     parameters->URI = strdup(path);
     if (!parameters->URI) {
         perror("Error: strdup() failed to copy URI");
         exit(EXIT_FAILURE);
     }

     /* Extract the http version */
     parameters->httpversion = strdup(saveptr);
     if (!parameters->httpversion) {
         perror("Error: strdup() failed to copy http version");
         exit(EXIT_FAILURE);
     }

     parameters->httpversion[strlen(saveptr)-1] = '\0';

     free(copy);
 }

 /* Checks if a given extension is served */
 /* Verifies that it is either .js, .jpg, .css or .html */
 static bool supported_file(const char *extension) {
     /* Go over the file types supported */
     for (size_t i = 0; i < ARRAY_LENGTH(file_map); i++) {

         /* If extension is the same here, return */
         if (strcmp(file_map[i].extension, extension) == 0) {
             return true;
         }
     }

     /* Otherwise, no matching extension was found */
     return false;
 }

 /* Gets full path of requested file */
 /* Return the absolute path */
 char *get_full_path(const char *path, const char *webroot, int *status) {
     char *full_path = NULL, *extension = NULL;

     /* Initialise reponse as not found */
     *status = NOT_FOUND;

     /* Create an array big enough for the web root and path */
     full_path = malloc(strlen(webroot) + strlen(path) + 1);
     if (!full_path) {
         perror("Error: malloc() failed allocate full path");
         exit(EXIT_FAILURE);
     }

     /* Combine web root and path */
     strcpy(full_path, webroot);
     strcat(full_path, path);

     /* Get string after last occurence of the dot character */
     extension = strrchr(full_path, '.');

     /* If extension is valid and path is accessible and file is supported */
     if (extension &&
         access(full_path, F_OK) == 0 &&
         supported_file(extension)) {

         /* update status to 200 */
         *status = FOUND;
     }

     /* Return the absolute path either way */
     return full_path;
 }

 /* Write 200 response headers */
 static void write_headers(int client, const char *data, const char *defaults) {
     char *buffer = NULL;

     /* Allocate big enough buffer */
     buffer = malloc(strlen(data) + strlen(defaults) + 1);
     if (!buffer) {
         perror("Error: malloc() failed to allocate buffer");
         exit(EXIT_FAILURE);
     }

     /* Write  data into buffer */
     sprintf(buffer, defaults, data);

     /* Write buffer to client socket */
     if (write(client, buffer, strlen(buffer)) == ERROR) {
         perror("Error: cannot write to socket");
         exit(EXIT_FAILURE);
     }

     /* Done with the buffer */
     free(buffer);

     return;
 }

 /* Calculates length of number */
 /* Returns count */
 static size_t get_length_bytes(size_t bytes) {
     size_t temp = bytes, count = 0;

     while (temp != 0) {
         temp /= 10;
         count++;
     }

     return count;
 }

 /* Write content length for requested file */
 /* Only applicable to 200 response headers */
 static void write_content_length(int client, size_t bytes_read) {
     char *content_length = NULL;
     size_t length_bytes, total_bytes;

     /* Get number of digits in bytes read */
     length_bytes = get_length_bytes(bytes_read);
     total_bytes = strlen(length_header) + length_bytes;

     /* Allocate buffer for content length */
     content_length = malloc(total_bytes + 1);
     if (!content_length) {
         perror("Error: malloc() failed to allocate content legnth");
         exit(EXIT_FAILURE);
     }

     /* Copy bytes read into buffer */
     snprintf(content_length, total_bytes + 1, "%zu", bytes_read);

     /* Write to client socket */
     write_headers(client, content_length, length_header);

     /* Done with this buffer */
     free(content_length);

     return;
 }

 /* Write file requested from 200 response */
 void read_write_file(int client, const char *path) {
     FILE *requested_file = NULL;
     unsigned char *buffer = NULL;
     size_t  bytes_read, buffer_size;
     long file_size;

     /* Open contents of file in binary mode */
     requested_file = fopen(path, "rb");
     if (!requested_file) {
         perror("Error: fopen() failed to open requested file");
         exit(EXIT_FAILURE);
     }

     /* Get size of file */
     fseek(requested_file, 0, SEEK_END);
     file_size = ftell(requested_file);
     fseek(requested_file, 0, SEEK_SET);

     /* Allocate buffer big enough to hold file */
     /* Since a file size can be anything, getting the file size beforehand -
        avoids the danger of buffer overflow */
     /* Having a sized buffer could be dangerous here */
     buffer_size = (size_t)file_size;
     buffer = malloc(buffer_size + 1);
     if (!buffer) {
         perror("Error: malloc() failed to allocate buffer");
         exit(EXIT_FAILURE);
     }

     /* Set everything to null terminating characters */
     /* Avoids having to null terminate the buffer later on */
     memset(buffer, '\0', buffer_size + 1);

     /* Write contents of file to buffer */
     bytes_read = fread(buffer, sizeof *buffer, file_size, requested_file);

     /* If the bytes is equal to the buffer size, fread was successful */
     if (bytes_read == buffer_size) {

         /* Write content length header */
         write_content_length(client, bytes_read);

         /* Write body of header to client socket */
         if (write(client, buffer, bytes_read) == ERROR) {
             perror("Error: cannot write to socket");
             exit(EXIT_FAILURE);
         }
     }

     /* Buffer has served its purpose, free it up */
     free(buffer);

     /* Close the file, just in case */
     fclose(requested_file);

     return;
 }

 void construct_file_response(int client, const char *httpversion,
                              const char *path, const char *status) {

     char *requested_file_extension = NULL;
     bool found = false;

     /* Write the status header */
     write_headers(client, httpversion, status);

     /* Get the file extension */
     requested_file_extension = strrchr(path, '.');

     /* If no extension exists, write appropriate response and exit */
     if (!requested_file_extension) {
         write(client, not_supported, strlen(not_supported));
         return;

     }

     /* otherwise, see if extension is served */
     for (size_t i = 0; i < ARRAY_LENGTH(file_map); i++) {
         if (strcmp(file_map[i].extension, requested_file_extension) == 0) {

             /* Write http content type */
             write_headers(client, file_map[i].mime_type, content_header);
             found = true;
             break;
         }
     }

     /* No extension was found, write not supported response */
     if (!found) {
         write(client, not_supported, strlen(not_supported));
     }

     return;
 }
