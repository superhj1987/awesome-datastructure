//
//  queue.h
//  elb-agent
//
//  Created by Bryant Hang on 14-6-6.
//
//

#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct queue_node node_t;
typedef struct queue      queue_t;

struct queue_node {
    node_t   *next;
};

struct queue {
    node_t   *tail;
    node_t    head;
};

void queue_init(queue_t *queue);
int queue_push(queue_t *queue, node_t *p);
node_t* queue_pop(queue_t *queue);
int queue_is_empty(queue_t *queue);

#define queue_pop_entry(queue, type, member) ({                             \
const typeof( ((type *)0)->member ) *__mptr = queue_pop(queue);     \
(__mptr == NULL) ? NULL : ((type *)((char *)__mptr - offsetof(type, member)));})


#endif