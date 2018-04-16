/* COMP30023 Computer Systems - Semester 1 2018
 * Assignment 1 - HTTP multi-threaded Web server
 * Author: Armaan Dhaliwal-McLeod
 * File: threadpool.c
 * Purpose: thread pool module. implements thread pool functions *.
 */

#include <stdio.h>
#include <stdlib.h>

#include "threadpool.h"

/* Create a new threadpool */
thread_pool *initialise_threadpool(workfunc_t work) {
    thread_pool *pool = NULL;

    /* Create thread pool */
    pool = malloc(sizeof *pool);
    if (!pool) {
        perror("Error: malloc() failed to create thread pool");
        exit(EXIT_FAILURE);
    }

    /* Initialise thread pool task queue */
    pool->task_queue = queue_new();

    /* Initialise thread pool mutex */
    if (pthread_mutex_init(&(pool->mutex), NULL)) {
        perror("Error: mutex init failed");
        exit(EXIT_FAILURE);
    }

    /* Initialise thread pool condition */
    if (pthread_cond_init(&(pool->cond), NULL)) {
        perror("Error: cond init failed");
        exit(EXIT_FAILURE);
    }

    /* Create workers for thread pool */
    create_workers(pool, MAX_THREADS);

    /* Add work function */
    pool->work = work;

    return pool;
}

/* Create workers here */
void create_workers(thread_pool *pool, size_t max_threads) {
    /* Create threadpool worker threads */
    for (size_t i = 0; i < max_threads; i++) {
        if (pthread_create(&(pool->threads[i]), NULL,
                           handle_client_request, pool)) {

            perror("Error: cannot create thread");
            exit(EXIT_FAILURE);
        }
    }

    return;
}

/* Add client work to task task queue */
void add_client_work(thread_pool *pool, int *client) {
    /* Critical section */
    pthread_mutex_lock(&(pool->mutex));

    /* Add client to the task_queue */
    queue_enqueue(pool->task_queue, client);

    pthread_mutex_unlock(&(pool->mutex));

    /* Send a signal to worker threads that a client task has been added */
    pthread_cond_signal(&(pool->cond));
}

/* Processes client request for a file */
void *handle_client_request(void *args) {
    void *socket = NULL;
    int client;

    /* Extract threadpool contents */
    thread_pool *pool = args;

    while (true) {
        /* Critical section */
        pthread_mutex_lock(&(pool->mutex));
        /* waiting for work to come up */
        while (queue_is_empty(pool->task_queue)) {
            pthread_cond_wait(&(pool->cond), &(pool->mutex));
        }

        /* deque first task */
        socket = queue_dequeue(pool->task_queue);

        pthread_mutex_unlock(&(pool->mutex));

        /* Get client socket id */
        client = *(int *)socket;

        /* process client task here */
        pool->work(client);

    }

    return NULL;

}

/* Clean up the thread pool */
void cleanup_pool(thread_pool *pool) {
    /* First unblock on threads */
    pthread_cond_broadcast(&(pool->cond));

    /* Free threads */
    for (size_t i = 0; i < MAX_THREADS; i++) {
        pthread_mutex_unlock(&(pool->mutex));
        pthread_cancel(pool->threads[i]);
        pthread_join(pool->threads[i], NULL);
    }

    /* Free up the the task_queue */
    queue_free(pool->task_queue);

    /* Destroy the mutex and conditions */
    pthread_mutex_destroy(&(pool->mutex));
    pthread_cond_destroy(&(pool->cond));

    /* Free up the thread pool */
    free(pool);

    return;
}
