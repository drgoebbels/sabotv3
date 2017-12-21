#include "log.h"
#include "pool.h"
#include "general.h"

typedef enum core_state_e core_state_e;

typedef struct core_s core_s;

enum core_state_e {
	CORE_STATE_ACTIVE,
	CORE_STATE_SHUTTING_DOWN,
	CORE_STATE_ZOMBIE
};

struct core_s {
	int id;
	pool_s *pool;
	core_state_e state;
	pthread_t thread;
};

struct pool_s {
	pool_task_s *head;
	pool_task_s *tail;
	int ncores;
	core_s cores[];
};

static void *core_thread(void *args);

int pool_init(int nthreads) {
	int i, result;	

	for(i = 0; i < nthreads; i++) {

	}
}

pool_task_s *pool_task_create(pool_s *pool, void *(*f)(void *), void *arg) {
	int result;
	pool_task_s *t;

	t = sa_alloc(sizeof *t);
	t->result = NULL;
	t->arg = arg;
	t->f = f;
	t->pool = pool;
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
	return t;
}

void *core_thread(void *args) {
}
