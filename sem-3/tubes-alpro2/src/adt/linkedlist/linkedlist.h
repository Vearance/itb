#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "../boolean.h"

typedef struct ListNode *Address;
typedef struct ListNode {
    int data;
    Address next;
} ListNode;
typedef Address List;
Address newNode(int data);


void createList(List *l);


void clearList(List *l);


int getElement(List l, int index);
void setElement(List *l, int index, int data);


boolean isEmpty(List l);


void insertFirst(List *l, int data);
void insertLast(List *l, int data);
void insertAt(List *l, int index, int data);
void deleteFirst(List *l, int *data);
void deleteLast(List *l, int *data);
void deleteAt(List *l, int index, int *data);
int length(List l);

void displayList(List l);



#endif
