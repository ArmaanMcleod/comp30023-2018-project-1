/* COMP30023 Computer Systems - Semester 1 2018
 * Assignment 1 - HTTP multi-threaded Web server
 * Author: Armaan Dhaliwal-McLeod
 * File: queue.c
 * Purpose: FIFO queue module. Queue functions implemented here.
 */

#include <stdlib.h>
#include <assert.h>

#include "queue.h"

/* Create new FIFO queue */
Queue *queue_new() {
    Queue * queue = malloc(sizeof *queue);
    assert(queue != NULL);

    queue->list = list_new();

    return queue;
}

/* Add item to queue */
void queue_enqueue(Queue *queue, void *data) {
    list_add_end(queue->list, data);
}

/* Remove item from queue */
void *queue_dequeue(Queue *queue) {
    return list_remove_start(queue->list);
}

/* Peek head of queue */
void *queue_peek(Queue *queue) {
    return list_head(queue->list);
}

/* Gets length of queue */
int queue_length(Queue *queue) {
    return list_length(queue->list);
}

/* Destroys entire queue list and queue pointer */
void queue_free(Queue *queue) {
    list_free(queue->list);
    free(queue);
}

/* Check if queue is not empty */
bool queue_is_empty(Queue *queue) {
    return list_is_empty(queue->list);
}
