#ifndef QUEUE_H
#define QUEUE_H

/* Import list functions */
#include "list.h"

/* Wrapper struct to hold queue, which is just a list */
typedef struct {
    List *list;
} Queue;

/* Create a new FIFO queue and return its pointer */
Queue *queue_new();

/* Add data to the front of the queue */
void queue_enqueue(Queue *queue, void *data);

/* Remove data at the end of the queue */
void *queue_dequeue(Queue *queue);

/* Return, but dont remove the head of the queue */
void *queue_peek(Queue *queue);

/* Get queue length */
int queue_length(Queue *queue);

/* Destroy the queue */
void queue_free(Queue *queue);

/* Check if queue is empty */
bool queue_is_empty(Queue *queue);

#endif
