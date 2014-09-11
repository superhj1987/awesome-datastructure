#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>

#include "tpool.h"

#define DEBUG

#if defined(DEBUG)
#define debug(...) do { \
    flockfile(stdout); \
    printf("###%p.%s: ", (void *)pthread_self(), __func__); \
    printf(__VA_ARGS__); \
    putchar('\n'); \
    fflush(stdout); \
    funlockfile(stdout); \
    } while (0)
#else
#define debug(...)
#endif

void *tpool_thread(void *);

void tpool_init(tpool_t *tpoolp, int num_worker_threads, int max_queue_size)
{
    int i;
    tpool_t pool;

    pool = (tpool_t)malloc(sizeof(struct tpool));
    if (pool == NULL) {
        perror("malloc");
        exit(0);
    }

    pool->num_threads = 0;
    pool->max_queue_size = max_queue_size + 1;
    pool->num_threads = num_worker_threads;
    pool->tpid = NULL;
    pool->front = 0;
    pool->rear = 0;
    pool->queue_closed = 0;
    pool->shutdown = 0;

    if (pthread_mutex_init(&pool->queue_lock, NULL) == -1) {
        perror("pthread_mutex_init");
        free(pool);
        exit(0);
    }
    if (pthread_cond_init(&pool->queue_has_space, NULL) == -1) {
        perror("pthread_mutex_init");
        free(pool);
        exit(0);
    }
    if (pthread_cond_init(&pool->queue_has_task, NULL) == -1) {
        perror("pthread_mutex_init");
        free(pool);
        exit(0);
    }
    if (pthread_cond_init(&pool->queue_empty, NULL) == -1) {
        perror("pthread_mutex_init");
        free(pool);
        exit(0);
    }

    if ((pool->queue = malloc(sizeof(struct tpool_work) *
                              pool->max_queue_size)) == NULL) {
        perror("malloc");
        free(pool);
        exit(0);
    }

    if ((pool->tpid = malloc(sizeof(pthread_t) * num_worker_threads)) == NULL) {
        perror("malloc");
        free(pool);
        free(pool->queue);
        exit(0);
    }

    for (i = 0; i < num_worker_threads; i++) {
        if (pthread_create(&pool->tpid[i], NULL, tpool_thread,
                           (void *)pool) != 0) {
            perror("pthread_create");
            exit(0);
        }
    }

    *tpoolp = pool;
}


int empty(tpool_t pool)
{
    return  pool->front == pool->rear;
}

int full(tpool_t pool)
{
    return ((pool->rear + 1) % pool->max_queue_size == pool->front);
}

int size(tpool_t pool)
{
    return (pool->rear + pool->max_queue_size -
            pool->front) % pool->max_queue_size;
}

int tpool_add_work(tpool_t tpool, void(*routine)(void *), void *arg)
{
    tpool_work_t *temp;

    pthread_mutex_lock(&tpool->queue_lock);

    while (full(tpool) && !tpool->shutdown && !tpool->queue_closed) {
        pthread_cond_wait(&tpool->queue_has_space, &tpool->queue_lock);
    }

    if (tpool->shutdown || tpool->queue_closed) {
        pthread_mutex_unlock(&tpool->queue_lock);
        return -1;
    }

    int is_empty = empty(tpool);

    temp = tpool->queue + tpool->rear;
    temp->routine = routine;
    temp->arg = arg;
    tpool->rear = (tpool->rear + 1) % tpool->max_queue_size;

    if (is_empty) {
        debug("signal has task");
        pthread_cond_broadcast(&tpool->queue_has_task);
    }

    pthread_mutex_unlock(&tpool->queue_lock);

    return 0;
}

void *tpool_thread(void *arg)
{
    tpool_t pool = (tpool_t)(arg);
    tpool_work_t *work;

    for (;;) {
        pthread_mutex_lock(&pool->queue_lock);

        while (empty(pool) && !pool->shutdown) {
            debug("I'm sleep");
            pthread_cond_wait(&pool->queue_has_task, &pool->queue_lock);
        }
        //        debug("I'm awake");

        if (pool->shutdown == 1) {
            debug("exit");
            pthread_mutex_unlock(&pool->queue_lock);
            pthread_exit(NULL);
        }

        int is_full = full(pool);
        work = pool->queue + pool->front;
        pool->front = (pool->front + 1) % pool->max_queue_size;

        if (is_full) {
            pthread_cond_broadcast(&pool->queue_has_space);
        }

        if (empty(pool)) {
            pthread_cond_signal(&pool->queue_empty);
        }

        pthread_mutex_unlock(&pool->queue_lock);

        (*(work->routine))(work->arg);
    }
}

int tpool_destroy(tpool_t tpool, int finish)
{
    int     i;

    pthread_mutex_lock(&tpool->queue_lock);

    tpool->queue_closed = 1;

    if (finish == 1) {
        debug("wait all work done");
        while (!empty(tpool)) {
            pthread_cond_wait(&tpool->queue_empty, &tpool->queue_lock);
        }
    }
    tpool->shutdown = 1;

    pthread_mutex_unlock(&tpool->queue_lock);

    pthread_cond_broadcast(&tpool->queue_has_task);

    debug("wait worker thread exit");
    for (i = 0; i < tpool->num_threads; i++) {
        pthread_join(tpool->tpid[i], NULL);
    }

    debug("free thread pool");
    free(tpool->tpid);
    free(tpool->queue);
    free(tpool);
}