#include "../pool.h"
#include "../general.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#define DEFAULT_TEST_SIZE 1000
#define DEFAULT_POOL_SIZE 50
#define NTEST_TYPES 5

static void *test1(void *args);
static void *test2(void *args);
static void *test3(void *args);
static void *test4(void *args);
static void *test5(void *args);

static void *(*jump_table[NTEST_TYPES])(void *);

int main(int argc, char *argv[]) {
	int test_size, pool_size;

	if(argc == 2) {
		test_size = atoi(argv[1]);
		pool_size = DEFAULT_POOL_SIZE;
	}
	else if(argc == 3) {
		test_size = atoi(argv[1]);
		pool_size = atoi(argv[2]);
	}
	else {
		test_size = DEFAULT_TEST_SIZE;
		pool_size = DEFAULT_POOL_SIZE;
	}

	long i;
	jump_table[0] = test1;	
	jump_table[1] = test2;	
	jump_table[2] = test3;	
	jump_table[3] = test4;	
	jump_table[4] = test5;	

	pool_task_s **tasks = sa_alloc(sizeof(*tasks) * test_size);
	pool_s *pool = pool_init(pool_size);
	
	for(i = 0; i < test_size; i++) {
		void *arg = (void *)i;
		tasks[i] = pool_task_create(
				pool, jump_table[rand() % NTEST_TYPES], arg
				);
	}	
	
	for(i = 0; i < test_size; i++) {
		pool_join_task(tasks[i]);
	}

	for(i = 0; i < test_size; i++) {
		void *value = (void *)i;
		printf("result: %p\n", tasks[i]->result);
		assert(value == tasks[i]->result);
		pool_destroy_task(tasks[i]);
	}

	pool_shutdown(pool);
	free(tasks);
	return 0;
}

void *test1(void *args) {
	return (args);
}

void *test2(void *args) {
	int i;
	for(i = 0; i < 1000; i++)
		sched_yield();
	puts("test 2");
	for(i = 0; i < 1000; i++)
		sched_yield();
	return (args);
}

void *test3(void *args) {
	int i;
	for(i = 0; i < 1000; i++)
		sched_yield();
	puts("test 3");
	return (args);
}

void *test4(void *args) {
	puts("test 4");
	sched_yield();
	puts("test 4");
	sched_yield();
	return (args);
}

void *test5(void *args) {
	puts("sleeping test 5");
	sleep(1);
	puts("test 5 done");
	sched_yield();
	return (args);
}

