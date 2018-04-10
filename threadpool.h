#ifndef POOL_H
#define POOL_H

#include <pthread.h>

#include "queue.h"

#define MAX_THREADS 100

typedef void (*workfunc_t)(int);

/* Thread pool information */
typedef struct {
    Queue *task_queue;
    pthread_t threads[MAX_THREADS];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    workfunc_t work;
} thread_pool;

thread_pool *initialise_threadpool(workfunc_t work);

void create_workers(thread_pool *pool, size_t max_threads);

void add_client_work(thread_pool *pool, int *client);

void *handle_client_request(void *args);

void cleanup_pool(thread_pool *pool);

#endif