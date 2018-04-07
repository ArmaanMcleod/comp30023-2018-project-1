#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"

/* Pointer to new linked list */
List *list_new() {
    List *list = malloc(sizeof *list);
    assert(list != NULL);

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

/* Add newnode to beginning of list */
void list_add_start(List *list, void *data) {
    assert(list != NULL && data != NULL);

    /* Create and initalise a new list node */
    Node *newnode = malloc(sizeof *newnode);
    assert(newnode != NULL);

    /* Allocate size for data to be stored in node */
    newnode->data = malloc(sizeof *data);
    assert(newnode->data != NULL);

    /* Safetly copy over node */
    memcpy(newnode->data, data, sizeof *data);

    /* Old head is now the next node */
    newnode->next = list->head;

    /* Head is now new node */
    list->head = newnode;

    if (list->size == 0) {
        list->tail = newnode;
    }

    list->size++;
}

/* Add new node to the end of a list */
void list_add_end(List *list, void *data) {
    assert(list != NULL && data != NULL);

    /* Create and initalise a new list node */
    Node *newnode = malloc(sizeof *newnode);
    assert(newnode != NULL);

    /* Allocate size for data to be stored in node */
    newnode->data = malloc(sizeof *data);
    assert(newnode->data != NULL);

    /* Safetly copy over node */
    memcpy(newnode->data, data, sizeof *data);

    /* Insert newnode data */
    newnode->next = NULL;

    /* If head is empty, this is the first newnode being added */
    if (list->size == 0) {
        list->head = newnode;

    /* Otherwise, add it to the tail */
    } else {
        list->tail->next = newnode;
    }

    /* New node is now last node */
    list->tail = newnode;

    /* Increment list size */
    list->size++;
}

/* Remove node from the head of the list */
void *list_remove_start(List *list) {
    assert(list != NULL && list->size > 0);

    /* Save the data to return it */
    Node *oldhead = list->head;
    void *data = oldhead->data;

    /* Replace the head with its next node */
    list->head = list->head->next;

    /* If this was the last node in the list */
    /* The last also needs to be cleared */
    if (list->size == 1) {
        list->tail = NULL;
    }

    /* Decrement list size */
    list->size--;

    /* Finished with this node */
    free(oldhead);

    return data;
}

/* Remove node from the tail of the list */ 
void *list_remove_end(List *list) {
    assert(list != NULL && list->size > 0);

    /* Save the data to return it */
    Node *oldtail = list->tail;
    void *data = oldtail->data;

    /* Replace the last with the second-last node */
    Node *curr = list->head;
    Node *prev = NULL;

    /* Walk down list to do replacement */
    while (curr != NULL && curr->next != NULL) {
        prev = curr;
        curr = curr->next;
    }
    list->tail = prev;

    if (list->size == 1) {
        /* Since removing the last node, the head also needs clearing */
        list->head = NULL;
    } else {
        prev->next = NULL;
    }

    /* Decrement list size */
    list->size--;

    /* Finished with this node */
    free(oldtail);

    return data;
}

/* Get the head of a list */
void *list_head(List *list) {
    return list->head;
}

/* Get the tail of a list */
void *list_tail(List *list) {
    return list->tail;
}

/* Gets size of linked list */
int list_length(List *list) {
    assert(list != NULL);
    return list->size;
}

/* Destroy linked list and free its memory */
void list_free(List *list) {
    assert(list != NULL);

    Node *curr = list->head;
    Node *prev = NULL;

    while (curr != NULL) {
        prev = curr;
        free(prev->data);
        free(prev);
        curr = curr->next;
    }

    free(list);
}

/* Check if list is empty */
bool list_is_empty(List *list) {
    assert(list != NULL);
    return (list->size == 0);
}





