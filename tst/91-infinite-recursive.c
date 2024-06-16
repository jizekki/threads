#include <stdio.h>
#include <assert.h>
#include "thread.h"

/**
 * An infinite recursive function  
 */
void *rec(void *retval)
{
    return rec(retval);
}

int main()
{
#ifdef STACK_PROTECT
    thread_t th;
    int err;
    void *res = NULL;
    err = thread_create(&th, rec, NULL);
    assert(!err);
    err = thread_join(th, &res);
    assert(!err);
#endif
}
