# COMP30023 Assignment 1
This repository contains Assignment 1 for COMP30023 Computer Systems Semester 1 2018. The aim of this project was to create a multi-threaded HTTP web server that can serve .js, .css, .html and .jpg files. A threadpool approach was used to handle multithreading.

## Files
* **test_script.sh** shell script that runs wget, grep, and diff to check HTTP responses to test HTTP GET request.
* **/test/** folder containing test files that will be served by your server.
* **server.c** main server program.
* **http.c/http.h** modules providing http handling of server.
* **threadpool.c/threadpool.h** modules providing threadpool implementation.
* **list.c/list.h** modules providing linked list implementation. taken from COMP20007 Design of Algorithms Sem 1 2017.
* **queue.c/queue.h** modules providing FIFO queue implementation. Functions use linked list functions from **list.c/list.h**.

## Running test script
Make sure that the **test_script.sh** is executable then run:

./test_script.sh *name_of_your_server* *port_number*

For example, if your server is compiled to *myserver*, run:

./test_script.sh myserver 8080

## Running server
Make sure you compile the server with either *make* or *make server*, then run:

./server *port_number* *web_root_path*

For example:

./server 8000 /home/ubuntu/website
