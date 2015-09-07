#include <stdio.h>
#include <pthread.h>
#include "tpool.h"

char *str[]={"string 0", "string 1", "string 2",
                "string 3", "string 4", "string 5"};

void job(void * jobstr)
{
    long i, x;

    for (i = 0; i < 100000000; i++)  {
        x = x +i;
    }
    printf("%s\n", (char *)jobstr);
}

int main(void)
{
    int i;
    tpool_t test_pool;

    tpool_init(&test_pool, 8, 20);

    for ( i = 0; i < 5; i++) {
        tpool_add_work(test_pool, job, str[i]);
    }

    tpool_destroy(test_pool, 1);

    return 0;
}
