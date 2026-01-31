#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../include/list.h"


static Node* createNode(ListElement value);


ListError listPushFront(List* list, ListElement value)
{
    assert(list);

    Node* new_node = createNode(value);
    if (new_node == NULL) {
        return LIST_OUT_OF_MEMORY;
    }

    new_node->next = list->head;
    list->head = new_node;
    list->size++;

    return LIST_OK;
}


ListError listConstructor(List** list)
{
    assert(list);

    *list = (List*)calloc(1, sizeof(List));
    if (*list == NULL) {
        return LIST_OUT_OF_MEMORY;
    }

    return LIST_OK;
}


void listDestructor(List* list)
{
    assert(list);

    Node* current = list->head;
    while (current) {
        Node* next = current->next;
        free(current);
        current = next;
    }

    list->head = NULL;
    list->size = 0;
}


static Node* createNode(ListElement data)
{
    Node* new_node = (Node*)calloc(1, sizeof(Node));
    if (new_node) {
        new_node->data = data;
        new_node->next = NULL;
    }

    return new_node;
}

