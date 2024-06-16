#include <stdlib.h>
#include "thread.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define MAX_SIZE 100

typedef struct table
{
    int start;
    int end;
    int *values;
} table;

typedef table *table_t;

void sum_table(table_t t)
{
    if (t->start == t->end)
    {
        thread_exit((void *)(intptr_t)(t->values[t->start]));
    }

    thread_t thread1, thread2;
    void *retval1, *retval2;
    table_t t1 = malloc(sizeof(table));
    table_t t2 = malloc(sizeof(table));

    t1->values = t->values;
    t1->start = t->start;
    t1->end = (t->start + t->end) / 2;

    t2->values = t->values;
    t2->start = (t->start + t->end) / 2 + 1;
    t2->end = t->end;

    int err = thread_create(&thread1, (void *(*)(void *))sum_table, (void *)t1);
    assert(!err);
    err = thread_create(&thread2, (void *(*)(void *))sum_table, (void *)t2);
    assert(!err);

    err = thread_join(thread2, &retval2);
    assert(!err);
    err = thread_join(thread1, &retval1);
    assert(!err);

    free(t1);
    free(t2);

    thread_exit((void *)((intptr_t)retval1 + (intptr_t)retval2));
}

int main(int argc, char *argv[])
{
    int max_size = MAX_SIZE;
    if (argc == 2)
    {
        max_size = atoi(argv[1]);
    }

    srand(time(NULL));
    int values[max_size];

    int i;
    int real_sum = 0;
    for (i = 0; i < max_size; i++)
    {
        values[i] = rand() % 100;
        real_sum += values[i];
    }

    table_t t = malloc(sizeof(table));
    thread_t thread;
    void *retval;
    struct timeval tv1, tv2;
    double s;

    t->values = values;
    t->start = 0;
    t->end = max_size - 1;

    gettimeofday(&tv1, NULL);
    int err = thread_create(&thread, (void *(*)(void *))sum_table, (void *)t);
    assert(!err);
    err = thread_join(thread, &retval);
    gettimeofday(&tv2, NULL);
    s = (tv2.tv_sec - tv1.tv_sec) * 1e6 + (tv2.tv_usec - tv1.tv_usec);

    assert((intptr_t)retval == real_sum);
    printf("calcul de %lu = %lu en %lu us\n", (long unsigned int)retval, (long unsigned int)real_sum, (long unsigned int)s);
    free(t);

    return EXIT_SUCCESS;
}
