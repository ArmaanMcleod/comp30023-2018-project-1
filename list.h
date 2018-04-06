#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

/* node that points to the next element in a list */
typedef struct node {
    void *data;
    struct node *next;
} Node;

/* list that points to its first and last items, and stores its size */
typedef struct {
    Node *head;
    Node *tail;
    int size;
} List;

/* Create a new linked list */
List *list_new();

/* Prepend item to the beginning og linked list */
void list_add_start(List *list, void *data);

/* Append item to end of list linked list */
void list_add_end(List *list, void *data);

/* Remove and get head of linked list */
void *list_remove_start(List *list);

/* Remove and get tail of linked list */
void *list_remove_end(List *list);

/* Get the head of the list */
void *list_head(List *list);

/* Get the tail of the list */
void *list_tail(List *list);

/* Get length of linked list */
int list_length(List *list);

/* Free linked list */
void list_free(List *list);

/* Check if list is empty */
bool list_is_empty(List *list);

#endif