/*
**队列实现**

对于数据节点，需要在数据结构体中加入一个类型为node_t的域；
然后使用下面的函数进行数据插入。
其中queue为队列指针，p为数据节点的node_t域，member为数据节点的node_t域的名字；type为数据节点类型；
 */

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