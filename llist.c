#include "llist.h"

llist *head = NULL, *tail = NULL;
// add node in linked list
void enqueue(int *socket)
{
    llist *node = malloc(sizeof(llist));
    node->data = socket;
    node->next = NULL;
    if (!tail)
    {
        head = node;
    }
    else
    {
        tail->next = node;
    }
    tail = node;
}

// remove node form a linked list
int *dequeue()
{
    if (!head)
    {
        return NULL;
    }
    int *out = head->data;
    llist *t = head;
    head = head->next;
    if (!head)
    {
        tail = NULL;
    }
    free(t);
    return out;
}