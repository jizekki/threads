#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <ucontext.h>
#include "queue.h"

#ifndef USE_PTHREAD

typedef enum thread_status
{
	RUNNING,
	JOIN,
	WAITING,
	FINISHED,
	DEAD,
} thread_state;

typedef struct thread_s
{
	ucontext_t uc;
	void *retval;
	struct thread_s *joiner;
	int state;
	int stack_id;
	STAILQ_ENTRY(thread_s)
	next;

} thread_s;

typedef thread_s *thread_t;

/**
 *  gets the current thread
 */
extern thread_t thread_self(void);

/**
 * creates a new thread that will execute the function func with the argument funcarg.
 * returns 0 in case of success, -1 in case of error.
 */
extern int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg);

/**
 * hand over to another thread
 */
extern int thread_yield(void);

/**
 * Wait untils the ends of execution of a thread
 * the value returned by the thread is placed in  *retval.
 * if retval is NULL, the returned value is ignored.
 */
extern int thread_join(thread_t thread, void **retval);

/** 
 * finish the current thread and sets the value of retval
 * this function never returns.
 *
 * The attrubute noreturn helps the compiler optimise the code of
 * the application (by clearing out dead code). Attention, do not put
 * this attribute in your interface as long as your thread_exit()
 * is not correctly implemented (it should never return).
 */
extern void thread_exit(void *retval) __attribute__((__noreturn__));

/** 
 * possible interface for mutex 
 */
typedef struct thread_mutex
{
	thread_t current_owner;
	STAILQ_HEAD(, thread_s)
	mutex_queue;
} thread_mutex_t;

int thread_mutex_init(thread_mutex_t *mutex);

int thread_mutex_destroy(thread_mutex_t *mutex);

int thread_mutex_lock(thread_mutex_t *mutex);

int thread_mutex_unlock(thread_mutex_t *mutex);

#else /* USE_PTHREAD */

/* Si on compile avec -DUSE_PTHREAD, ce sont les pthreads qui sont utilisés */
#include <sched.h>
#include <pthread.h>
#define thread_t pthread_t
#define thread_self pthread_self
#define thread_create(th, func, arg) pthread_create(th, NULL, func, arg)
#define thread_yield sched_yield
#define thread_join pthread_join
#define thread_exit pthread_exit

/* Interface possible pour les mutex */
#define thread_mutex_t pthread_mutex_t
#define thread_mutex_init(_mutex) pthread_mutex_init(_mutex, NULL)
#define thread_mutex_destroy pthread_mutex_destroy
#define thread_mutex_lock pthread_mutex_lock
#define thread_mutex_unlock pthread_mutex_unlock

#endif /* USE_PTHREAD */

#endif //THREADS_THREAD_H
