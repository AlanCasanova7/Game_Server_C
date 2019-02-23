#include "queue.h"
queue_t* new_queue(){
    queue_t* to_return = malloc(sizeof(queue_t));
    to_return->head = NULL;
    to_return->tail = NULL;
    to_return->elements = 0;
    return to_return;
}

void enqueue(queue_t* queue, void* element){
    queue_element_t* element_to_add = new_q_element(element);
    if (queue->elements > 1)
    {
        queue->tail->next = element_to_add;
        queue->tail = element_to_add;
    }
    else if(queue->elements == 1)
    {
        queue->head->next = element_to_add;
        queue->tail = element_to_add;
    }
    else
    {
        queue->head = element_to_add;
    }

    queue->elements += 1;
}

void* dequeue(queue_t* queue){
    void *to_return = NULL;
    if (queue->elements > 1)
    {
        to_return = queue->head->element;
        queue->head = queue->head->next;
        return to_return;
    }
    else if(queue->elements == 1)
    {
        to_return = queue->head->element;
        queue->head = NULL;
        queue->elements -= 1;
        return to_return;
    }
    else
    {
        return to_return;
    }
}

static queue_element_t* new_q_element(void* element){
    queue_element_t *element_to_return = malloc(sizeof(queue_element_t));
    element_to_return->element = element;
    element_to_return->next = NULL;
    return element_to_return;
}