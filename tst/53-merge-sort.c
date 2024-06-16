#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include "thread.h"

#define TAB_SIZE 100

struct param
{
    int *tab;
    int i;
    int j;
    int *tmp;
};

static void *merge_sort(void *param)
{
    struct param *param0 = (struct param *)param;
    int *tab = param0->tab;
    int i = param0->i;
    int j = param0->j;
    int *tmp = param0->tmp;
    if (j <= i)
        return NULL;
    int m = (i + j) / 2;

    struct param param1 = {tab, i, m, tmp};
    struct param param2 = {tab, m + 1, j, tmp};

    thread_t thread_tab[2];
    thread_create(&thread_tab[0], merge_sort, &param1);
    thread_create(&thread_tab[1], merge_sort, &param2);

    thread_join(thread_tab[0], NULL);
    thread_join(thread_tab[1], NULL);

    int pg = i;
    int pd = m + 1;
    int c;

    for (c = i; c <= j; c++)
    {
        if (pg == m + 1)
        {
            tmp[c] = tab[pd];
            pd++;
        }
        else if (pd == j + 1)
        {
            tmp[c] = tab[pg];
            pg++;
        }
        else if (tab[pg] < tab[pd])
        {
            tmp[c] = tab[pg];
            pg++;
        }
        else
        {
            tmp[c] = tab[pd];
            pd++;
        }
    }
    for (c = i; c <= j; c++)
    {
        tab[c] = tmp[c];
    }
    return NULL;
}

void print_tab(int *tab, int size)
{
    printf("[");
    int i;
    for (i = 0; i < size; i++)
    {
        printf("%d, ", tab[i]);
    }
    printf("]");
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("Size of sorted array needed\n");
        exit(1);
    }
    int n = atoi(argv[1]);
    int *tab = malloc(n * sizeof(int));
    int i, j;
    unsigned long us;
    for (i = 0; i < n; i++)
    {
        if (i % 2 == 0)
            tab[i / 2] = i;
        else
            tab[n / 2 + i / 2] = i;
    }

    int *tmp = malloc(n * sizeof(int));
    struct param param3 = {tab, 0, n - 1, tmp};

    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
    merge_sort(&param3);
    gettimeofday(&tv2, NULL);
    for (j = 0; j < n - 1; j++)
    {
        if (tab[j] > tab[j + 1])
        {
            printf("Tab is not sorted\n");
            exit(1);
        }
    }
    us = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
    printf("Time : %lu us\n", us);
    free(tab);
    free(tmp);
}
