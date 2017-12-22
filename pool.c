#include "log.h"
#include "pool.h"
#include "general.h"
#include <pthread.h>

typedef enum pool_state_e pool_state_e;

typedef struct core_s core_s;

enum pool_state_e {
	POOL_STATE_ACTIVE,
	POOL_STATE_FLUSH,
	POOL_STATE_SHUTDOWN,
};

struct core_s {
	int id;
	pool_s *pool;
	pthread_t thread;
};

struct pool_s {
	pool_state_e state;
	pthread_mutex_t qlock;
	pthread_cond_t qcond;
	pool_task_s *head;
	pool_task_s *tail;
	int ncores;
	core_s cores[];
};

static void *core_thread(void *args);

pool_s *pool_init(int nthreads) {
	int i, result;	
	pool_s *pool;
	core_s *core;

	pool = sa_alloc(sizeof(*pool) + nthreads*sizeof(*pool->cores));
	pool->state = POOL_STATE_ACTIVE;
	pool->head = NULL;
	pool->tail = NULL;
	pool->ncores = nthreads;
	result = pthread_mutex_init(&pool->qlock, NULL);
	if(result) {
		log_error_errno("Failed to create thread pool - pthread_mutex_init() failed");
		free(pool);
		return NULL;
	}
	result = pthread_cond_init(&pool->qcond, NULL); 
	if(result) {
		log_error_errno("Failed to create thread pool - pthread_cond_init() failed");
		pthread_mutex_destroy(&pool->qlock);
		free(pool);
		return NULL;
	}
	for(i = 0, core = pool->cores; i < nthreads; i++) {
		core[i].id = i;	
		core[i].pool = pool;
		result = pthread_create(&core[i].thread, NULL, core_thread, &core[i]);
		if(result) {
			log_error_errno("Failed to create thread %d", i);
			free(pool);
		}
	}
	return pool;
}

pool_task_s *pool_task_create(pool_s *pool, void *(*f)(void *), void *arg) {
	int result;
	pool_task_s *t;

	if(pool->state != POOL_STATE_ACTIVE) {
		return NULL;
	}
	t = sa_alloc(sizeof *t);
	t->isfinished = false;
	t->result = NULL;
	t->arg = arg;
	t->f = f;
	t->pool = pool;
	t->next = NULL;
	result = pthread_mutex_init(&t->lock, NULL);
	if(result) {
		log_error_errno("Failed to create thread pool task for pool: %p. Failed to initialize mutex", pool);
		free(t);
		return NULL;
	}
	result = pthread_cond_init(&t->cond, NULL);
	if(result) {
		log_error_errno("Failed to create thread pool task for pool: %p. Failed to initialize condition variable", pool);
		pthread_mutex_destroy(&t->lock);
		free(t);
		return NULL;
	}
	pthread_mutex_lock(&pool->qlock);
	if(pool->tail) {
		pool->tail->next = t;
	}
	else {
		pool->head = t;
	}
	pool->tail = t;
	pthread_cond_signal(&pool->qcond);
	pthread_mutex_unlock(&pool->qlock);
	return t;
}

int pool_join_task(pool_task_s *task) {
	if(task->isfinished)
		return 0;
	if(task->pool->state == POOL_STATE_SHUTDOWN) {
		return -1;
	}
	pthread_mutex_lock(&task->lock);
	while(!task->isfinished)
		pthread_cond_wait(&task->cond, &task->lock);
	pthread_mutex_unlock(&task->lock);
	return 0;
}

void pool_destroy_task(pool_task_s *task) {
	pthread_mutex_destroy(&task->lock);
	pthread_cond_destroy(&task->cond);
	free(task);
}

void *core_thread(void *args) {
	core_s *core = args;
	pool_s *pool = core->pool;
	pool_task_s *task;

	log_info("Starting up core thread: %d.", core->id);
	while(true) {
		pthread_mutex_lock(&pool->qlock);
		while(!pool->head && pool->state == POOL_STATE_ACTIVE) 
			pthread_cond_wait(&pool->qcond, &pool->qlock);	
		if(pool->head) {
			task = pool->head;
			pool->head = pool->head->next;
			if(!pool->head)
				pool->tail = NULL;
			pthread_mutex_unlock(&pool->qlock);
			pthread_mutex_lock(&task->lock);
			task->result = task->f(task->arg);
			task->isfinished = true;
			pthread_cond_signal(&task->cond);
			pthread_mutex_unlock(&task->lock);
		}
		else {
			log_info("Shutting down core thread: %d.", core->id);
			pthread_mutex_unlock(&pool->qlock);
			pthread_exit(NULL);
		}
	}
}

void pool_shutdown(pool_s *pool) {
	int i, resut;
	bool alldone = false; 
	core_s *cores = pool->cores;
		
	log_info("calling pool_shutdown()");
	pthread_mutex_lock(&pool->qlock);
	pool->state = POOL_STATE_FLUSH;
	while(pool->head)
		pthread_cond_wait(&pool->qcond, &pool->qlock);

	log_info("pool is empty");

	pool->state = POOL_STATE_SHUTDOWN;
	pthread_cond_broadcast(&pool->qcond);
	pthread_mutex_unlock(&pool->qlock);

	for(i = 0; i < pool->ncores; i++ ) {
		pthread_join(cores[i].thread, NULL);
	}
	pthread_mutex_destroy(&pool->qlock);
	pthread_cond_destroy(&pool->qcond);
	free(pool);	
}

