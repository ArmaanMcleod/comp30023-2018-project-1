#include <stdio.h>
#include <stdlib.h>

#include "list.h"

void print_int(void *n) {
    printf("%d\n", *(int*) n);
}

void print_list(List *list, void(*f)(void*)) {
    Node *curr = list->head;

    while (curr != NULL) {
        f(curr->data);
        curr = curr->next;
    }
}

int main(void) {
    List *newlist = list_new();

    for (int i = 1; i < 4; i++) {
        list_add_start(newlist, &i);
    }

    for (int i = 1; i < 4; i++) {
        list_add_end(newlist, &i);
    }

    print_list(newlist, print_int);

    void *start = list_remove_start(newlist);
    printf("\n%d\n", *(int*)start);

    void *end = list_remove_end(newlist);
    printf("%d\n\n", *(int*)end);

    print_list(newlist, print_int);

    return 0;
}

