#ifndef __pool_h__
#define __pool_h__

#include <pthread.h>

typedef struct pool_task_s pool_task_s;
typedef struct pool_s pool_s;

struct pool_task_s {
	void *result;
	void *arg;
	void *(*f)(void *);
	pool_s *pool;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	pool_task_s *next;
};

extern int pool_init(int nthreads);
extern pool_task_s *pool_task_create(pool_s *pool, void *(*f)(void *), void *arg);

#endif

