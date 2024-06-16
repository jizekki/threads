#include <stdio.h>
#include <assert.h>
#include "thread.h"

/**
 *  Access to a non allocated pointer 
 */
void *access(void *retval)
{
    int *t;
    *t = 0;
    return t;
}

int main()
{
#ifdef STACK_PROTECT
        thread_t th;
    int err;
    void *res = NULL;
    err = thread_create(&th, access, NULL);
    assert(!err);
    err = thread_join(th, &res);
    assert(!err);
#endif
}
