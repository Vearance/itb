#include "linkedlist.h"

#include <stdio.h>
#include <stdlib.h>

Address newNode(int data) {
    Address p = (Address) malloc(sizeof(ListNode));
    if (p!=NULL) {
        p->data = data;
        p->next = NULL;
    }
    return p;
}

void createList(List *l) {
    *l = NULL;
}

void clearList(List *l) {
    Address p = *l;
    Address temp;
    while (p != NULL) {
        temp = p;
        p = p->next;
        free(temp);
    }
    *l = NULL;
}

int getElement(List l, int index) {
    Address p = l;
    int i = 0;
    while (p != NULL && i < index) {
        p = p->next;
        i++;
    }
    if (p == NULL) {
        fprintf(stderr, "LinkedList: getElement index out of range (%d)\n", index);
        exit(EXIT_FAILURE);
    }
    return p->data;
}

void setElement(List *l, int index, int data) {
    Address p = *l;
    int i = 0;
    while (p != NULL && i < index) {
        p = p->next;
        i++;
    }
    if (p == NULL) {
        fprintf(stderr, "LinkedList: setElement index out of range (%d)\n", index);
        exit(EXIT_FAILURE);
    }
    p->data = data;
}

boolean isEmpty(List l) {
    return l == NULL;
}

void insertFirst(List *l, int data) {
    Address p = newNode(data);
    if (p == NULL) {
        fprintf(stderr, "LinkedList: failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    p->next = *l;
    *l = p;
}

void insertLast(List *l, int data) {
    Address p = newNode(data);
    if (p == NULL) {
        fprintf(stderr, "LinkedList: failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    if (isEmpty(*l)) {
        *l = p;
    } 
    else {
        Address last = *l;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = p;
    }
}

void insertAt(List *l, int index, int data) {
    if (index == 0) {
        insertFirst(l, data);
        return;
    }

    Address p = *l;
    int i = 0;
    while (p != NULL && i < index - 1) {
        p = p->next;
        i++;
    }
    if (p == NULL) {
        fprintf(stderr, "LinkedList: insertAt index out of range (%d)\n", index);
        exit(EXIT_FAILURE);
    }

    Address new_node = newNode(data);
    if (new_node == NULL) {
        fprintf(stderr, "LinkedList: failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    new_node->next = p->next;
    p->next = new_node;
}

void deleteFirst(List *l, int *data) {
    if (isEmpty(*l)) {
        fprintf(stderr, "LinkedList: deleteFirst from empty list\n");
        exit(EXIT_FAILURE);
    }

    Address p = *l;
    *data = p->data;
    *l = p->next;
    free(p);
}

void deleteLast(List *l, int *data) {
    if (isEmpty(*l)) {
        fprintf(stderr, "LinkedList: deleteLast from empty list\n");
        exit(EXIT_FAILURE);
    }

    Address p = *l;
    if (p->next == NULL) {
        *data = p->data;
        *l = NULL;
        free(p);
        return;
    }

    Address prev = NULL;
    while (p->next != NULL) {
        prev = p;
        p = p->next;
    }
    *data = p->data;
    prev->next = NULL;
    free(p);
}

void deleteAt(List *l, int index, int *data) {
    if (isEmpty(*l)) {
        fprintf(stderr, "LinkedList: deleteAt from empty list\n");
        exit(EXIT_FAILURE);
    }

    if (index == 0) {
        deleteFirst(l, data);
        return;
    }

    Address p = *l;
    Address prev = NULL;
    int i = 0;
    while (p != NULL && i < index) {
        prev = p;
        p = p->next;
        i++;
    }
    if (p == NULL) {
        fprintf(stderr, "LinkedList: deleteAt index out of range (%d)\n", index);
        exit(EXIT_FAILURE);
    }

    *data = p->data;
    prev->next = p->next;
    free(p);
}

int length(List l) {
    Address p = l;
    int count = 0;
    while (p != NULL) {
        count++;
        p = p->next;
    }
    return count;
}


void displayList(List l) {
    Address p = l;
    printf("[");
    while (p != NULL) {
        printf("%d", p->data);
        p = p->next;
        if (p != NULL) {
            printf(", ");
        }
    }
    printf("]\n");
}
