#include "thread.h"
#include <stdlib.h>
#include <valgrind/valgrind.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <sys/mman.h>
#include <malloc.h>
#include <unistd.h>
#define DEBUG 0

#if DEBUG
#define THREAD_DEBUG_OUT(...)                 \
	do                                        \
	{                                         \
		fprintf(stderr, "DEBUG:"__VA_ARGS__); \
	} while (0)
#else
#define THREAD_DEBUG_OUT(...)
#endif //DEBUG

#define THREAD_STACK_SIZE 1024 * 64 // to allocate a 64kB stack
STAILQ_HEAD(, thread_s)
ready_threads_queue;

thread_t current_thread = NULL;
thread_t main_thread = NULL;
thread_t to_free = NULL;

#ifdef STACK_PROTECT
/**
 * Handle The SIGSEV signal when the thread queue is Overwhelmed
 */
void stack_handler()
{
	fprintf(stderr, "OUT OF STACK \n");
	thread_exit(NULL);
}

/**
 * Display error message and exit with a failure 
 */
void exit_error(const char *err)
{
	perror(err);
	exit(EXIT_FAILURE);
}
#endif

static inline void insert_thread_to_running_queue(thread_t thread)
{
	STAILQ_INSERT_TAIL(&ready_threads_queue, thread, next);
}

static inline void remove_thread_from_running_queue(thread_t thread)
{
	STAILQ_REMOVE(&ready_threads_queue, thread, thread_s, next);
}

static inline void *run_thread(void *(*func)(void *), void *funcarg)
{
	thread_exit(func(funcarg));
	return NULL;
}

static inline void free_thread(thread_t thread)
{
	if (thread != NULL && thread != main_thread)
	{
		VALGRIND_STACK_DEREGISTER(thread->stack_id);
		free(thread->uc.uc_stack.ss_sp);
		free(thread);
		thread = NULL;
	}
}

/**
 *  gets the current thread
 */
inline thread_t thread_self(void)
{
	return current_thread;
}

/**
 * creates a new thread that will execute the function func with the argument funcarg.
 * returns 0 in case of success, -1 in case of error.
 */
int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg)
{
	THREAD_DEBUG_OUT("entering create\n");
	if (newthread == NULL)
		return -1;

	// Memory allocation and initialization of the thread
	(*newthread) = (thread_t)malloc(sizeof(thread_s));
	(*newthread)->joiner = NULL;
	(*newthread)->state = RUNNING;
	getcontext(&((*newthread)->uc));
#ifndef STACK_PROTECT
	(*newthread)->uc.uc_stack.ss_size = THREAD_STACK_SIZE;
	(*newthread)->uc.uc_stack.ss_sp = malloc((*newthread)->uc.uc_stack.ss_size);
	(*newthread)->stack_id = VALGRIND_STACK_REGISTER((*newthread)->uc.uc_stack.ss_sp, (*newthread)->uc.uc_stack.ss_sp + (*newthread)->uc.uc_stack.ss_size);
	(*newthread)->uc.uc_link = NULL;
#else

	size_t pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1)
		exit_error("sysconf");

	(*newthread)->uc.uc_stack.ss_size = THREAD_STACK_SIZE;
	
	if (posix_memalign(&(*newthread)->uc.uc_stack.ss_sp, pagesize, (*newthread)->uc.uc_stack.ss_size+pagesize) == -1)
	{
		exit_error("error posix_memalign");
	}

	if (mprotect((*newthread)->uc.uc_stack.ss_sp+THREAD_STACK_SIZE, pagesize, PROT_NONE) == -1)
	{
		exit_error("error mprotect");
	}
	(*newthread)->stack_id = VALGRIND_STACK_REGISTER((*newthread)->uc.uc_stack.ss_sp, (*newthread)->uc.uc_stack.ss_sp + (*newthread)->uc.uc_stack.ss_size);
	(*newthread)->uc.uc_link = NULL;

	struct sigaction sa;
	sa.sa_handler = stack_handler;

	stack_t ss;

	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = 0;
	
	ss.ss_sp = mmap(NULL, SIGSTKSZ, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (ss.ss_sp == MAP_FAILED)
	{
		exit_error("error mmap");
	}
	if (sigaltstack(&ss, NULL) == -1)
	{
		exit_error("error sigaltstack");
	}
	sigemptyset(&sa.sa_mask);

	sa.sa_flags = SA_ONSTACK;
	if (sigaction(SIGSEGV, &sa, NULL) == -1)
	{
		exit_error("error sigaction");
	}
#endif
	makecontext(&((*newthread)->uc), (void (*)(void))run_thread, 2, (void (*)(void))func, funcarg);
	insert_thread_to_running_queue(*newthread);
	thread_yield();
	return 0;
}

/**
 * hand over to another thread
 */
int thread_yield(void)
{
	thread_t old_thread = current_thread;
	thread_t next_thread = main_thread;
	switch (old_thread->state)
	{
		case FINISHED:
			if (old_thread->joiner != NULL && old_thread->joiner->state == JOIN) // Allow the joiner to run
			{
				old_thread->state = WAITING;
				insert_thread_to_running_queue(old_thread->joiner);
			}
			old_thread->state = DEAD;
			break;
		case RUNNING:
			insert_thread_to_running_queue(old_thread);
			break;
		default:
			break;
	}
	if (!STAILQ_EMPTY(&ready_threads_queue))
	{
		next_thread = STAILQ_FIRST(&ready_threads_queue);
		STAILQ_REMOVE_HEAD(&ready_threads_queue, next);
	}
	current_thread = next_thread;
	swapcontext(&(old_thread->uc), &(next_thread->uc));
	return 0;
}

/**
 * Wait untils the ends of execution of a thread
 * the value returned by the thread is placed in  *retval.
 * if retval is NULL, the returned value is ignored.
 */
int thread_join(thread_t thread, void **retval)
{
	THREAD_DEBUG_OUT("entering join\n");
	
	/* No thread to join */
	if (thread == NULL)
		return -1;
	
	/* Currently joined by someone else */
	if (thread->joiner != NULL && thread->joiner != current_thread)
		return -1;
	
	/* Already completed */
	if (thread->state == DEAD)
	{
		if (retval != NULL)
		{
			*retval = thread->retval;
		}
		free_thread(thread);
		return 0;
	}

	/* Wait for its completion before continuing */
	thread->joiner = current_thread;
	current_thread->state = JOIN;
	thread_yield();

	/* Completed, set the return value */
	if (retval != NULL)
	{
		*retval = thread->retval;
	}

	/* Free used memory */
	free_thread(thread);

	return 0;
}

/**
 * finish the current thread and sets the value of retval
 * this function never returns.
 *
 * The attrubute noreturn helps the compiler optimise the code of
 * the application (by clearing out dead code). Attention, do not put
 * this attribute in your interface as long as your thread_exit()
 * is not correctly implemented (it should never return).
 */
void thread_exit(void *retval)
{
	THREAD_DEBUG_OUT("entering thread_exit\n");
	current_thread->retval = retval;
	current_thread->state = FINISHED;
	to_free = current_thread;
	thread_yield();
	free_thread(to_free);
	exit(EXIT_SUCCESS);
}

int thread_mutex_init(thread_mutex_t *mutex)
{
	if (mutex)
	{
		STAILQ_INIT(&(mutex->mutex_queue));
		return 0;
	}
	return -1;
}

int thread_mutex_destroy(thread_mutex_t *mutex)
{
	return 0;
}

int thread_mutex_lock(thread_mutex_t *mutex)
{
	THREAD_DEBUG_OUT("entering lock\n");
	if (mutex->current_owner == NULL)
	{
		mutex->current_owner = current_thread;
		return 0;
	}
	STAILQ_INSERT_TAIL(&(mutex->mutex_queue), current_thread, next);
	current_thread->state = WAITING;
	thread_yield();
	return 1;
}

int thread_mutex_unlock(thread_mutex_t *mutex)
{
	THREAD_DEBUG_OUT("entering unlock\n");
	if (mutex->current_owner == current_thread)
	{
		mutex->current_owner = NULL;
		if (!STAILQ_EMPTY(&(mutex->mutex_queue)))
		{
			thread_t next_thread = STAILQ_FIRST(&(mutex->mutex_queue));
			STAILQ_REMOVE_HEAD(&(mutex->mutex_queue), next);
			mutex->current_owner = next_thread;
			next_thread->state = RUNNING;
			STAILQ_INSERT_TAIL(&ready_threads_queue, next_thread, next);
		}
		return 0;
	}
	return -1;
}

/**
 * called at the beginning of the program while loading the library.
 * initializes main_thread and sets the current thread to the main thread
 */
static void load_thread(void) __attribute__((constructor));
static inline void load_thread(void)
{
	THREAD_DEBUG_OUT("loading library\n");

	STAILQ_INIT(&ready_threads_queue);
	main_thread = (thread_t)malloc(sizeof(thread_s));
	main_thread->state = RUNNING;
	main_thread->joiner = NULL;
	getcontext(&(main_thread->uc));
	current_thread = main_thread;
}

/**
 * called at the end of the program.
 * frees the remaining allocated memory (main_thread)
 */
static void unload_thread(void) __attribute__((destructor));
static inline void unload_thread(void)
{
	free(main_thread);
	assert(STAILQ_EMPTY(&ready_threads_queue));
}
