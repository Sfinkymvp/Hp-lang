#ifndef LIST_H_
#define LIST_H_


#include <stdio.h>


typedef void* ListElement;


typedef enum {
    LIST_OK,
    LIST_OUT_OF_MEMORY
} ListError;


typedef struct Node {
    ListElement data;
    struct Node* next;
} Node;


typedef struct {
    Node* head;
    size_t size;
} List;


ListError listPushFront(List* list, ListElement data);


ListError listConstructor(List** list);


void listDestructor(List* list);


#endif // LIST_H_