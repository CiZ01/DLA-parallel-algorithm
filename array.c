#include <stdio.h>
#include <stdlib.h>

typedef struct node
{
    int value;
    struct node *Next;
} node;

typedef struct
{
    node *head;
    node *tail;
    int size;
} list;
