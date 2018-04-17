/* COMP30023 Computer Systems - Semester 1 2018
 * Assignment 1 - HTTP multi-threaded Web server
 * Author: Armaan Dhaliwal-McLeod
 * File: threadpool.h
 * Purpose: thread pool header file. Defines contents of thread pool, which -
            is used for multi-threading
 */

#ifndef POOL_H
#define POOL_H

#include <pthread.h>

#include "queue.h"

#define MAX_THREADS 100

/* Function pointer used to reference process work function in server */
typedef void (*workfunc_t)(int);

/* Thread pool information */
typedef struct {
    /* Queue for holding client tasks */
    Queue *task_queue;

    /* Worker threads */
    pthread_t threads[MAX_THREADS];
    size_t num_threads;

    /* Mutex variables and conditions */
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    /* Work function */
    workfunc_t work;
} thread_pool;

/* Create a thread pool */
thread_pool *initialise_threadpool(workfunc_t work);

/* Create worker threads */
void create_workers(thread_pool *pool);

/* Add client to stack */
void add_client_work(thread_pool *pool, int *client);

/* Process a client task */
void *handle_client_request(void *args);

/* Destroy thread pool */
void cleanup_pool(thread_pool *pool);

#endif
