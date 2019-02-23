#include <stdlib.h>

typedef struct queue_element
{
    void* element;
    struct queue_element* next;
} queue_element_t;

typedef struct queue{
    int elements;
    queue_element_t* head;
    queue_element_t* tail;
} queue_t;

queue_t* new_queue();
void enqueue(queue_t* queue, void* element);
void* dequeue(queue_t* queue);
queue_element_t* new_q_element(void* element);