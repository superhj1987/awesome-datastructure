/*
 * Queue implementation.
 */
#include "queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void queue_init(queue_t *queue)
{
    queue->head.next = NULL;
    queue->tail = &queue->head;
}

int queue_push(queue_t *queue, node_t *p)
{
    node_t *tail;
    do {
        tail = queue->tail;
    } while (__sync_bool_compare_and_swap(&tail->next, NULL, p) == 0);
    
    __sync_bool_compare_and_swap(&queue->tail, tail, p);
    return 0;
}

node_t* queue_pop(queue_t *queue)
{
    node_t *p;
    node_t *dummy = NULL;
    do {
        if((p = queue->head.next) == NULL)
            return NULL;
    } while (__sync_bool_compare_and_swap(&queue->head.next, p, p->next) == 0);
    //recover
    if (queue->head.next == NULL) {
        if (__sync_bool_compare_and_swap(&p->next, NULL, dummy) == 0) {
            //已经有元素插入
            queue->head.next = p->next;
        } else {
            //此时tail指向的链一定都已经被pop出去
            //p->next非空，push操作将会死循环
            queue->tail = &queue->head;
        }
    }
    return p;
}

int queue_is_empty(queue_t *queue){
    return &queue->head == queue->tail;
}