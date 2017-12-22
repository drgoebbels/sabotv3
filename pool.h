#ifndef __pool_h__
#define __pool_h__

#include <pthread.h>
#include <stdbool.h>

typedef struct pool_task_s pool_task_s;
typedef struct pool_s pool_s;

struct pool_task_s {
	bool isfinished;
	void *result;
	void *arg;
	void *(*f)(void *);
	pool_s *pool;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	pool_task_s *next;
};

extern pool_s *pool_init(int nthreads);
extern pool_task_s *pool_task_create(pool_s *pool, void *(*f)(void *), void *arg);
extern int pool_join_task(pool_task_s *task);
extern void pool_destroy_task(pool_task_s *task);
extern void pool_shutdown(pool_s *pool);

#endif

