#include <stdio.h>
#include "queue.h"

typedef struct cmd_s cmd_t;

struct cmd_s{
    node_t queue;
    char   cmd_id[1024];
    int    type;
    int    result;
    char   operation[1024];
    char   route[1024];
    char   arg[16000];
    int    interval;
} ;

int main(void)
{
    queue_t cmd_task_queue;
    queue_init(&cmd_task_queue);

    cmd_t *cmd = (cmd_t *)malloc(sizeof(cmd_t));
    bzero(cmd_,sizeof(cmd_t));

    queue_push(&cmd_task_queue, &(cmd->queue));
    cmd = queue_pop_entry(&cmd_task_queue, cmd_t, queue); 
}
