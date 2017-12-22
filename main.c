#include "log.h"
#include "pool.h"
#include "server.h"
#include "socks5.h"
#include <stdio.h>
#include <unistd.h>

#define LISTEN_PORT 5454
#define PROXY_SERVER "127.0.0.1"
#define PROXY_PORT 9050

static void shutdown(void);

void *test_task(void *arg) {
	printf("hello from task: %p\n", arg);
	return arg;
}

int main(void) {
	int result, fd; 
	pool_s *pool;
	pool_task_s *tasks[500];

	log_init_name("sabot.log");
	log_puts("---------------------------------------------------------");
	log_info("Starting SA Bot V3");
	pool = pool_init(10);
	if(!pool) {
		log_error("Failed to start thread pool.");
	}

	result = server_start(LISTEN_PORT);
	if(result < 0) {
		log_error("Failed to start server on port %d.", LISTEN_PORT);
	}
	fd = socks5_connect(PROXY_SERVER, PROXY_PORT);
	if(fd >= 0) {
		log_info("Successfull connection: Got fd: %d", fd);
	}
	else {
		log_error("Unsuccessful connection: Got fd: %d", fd);
	}
	
	log_info("task testing");
	long i;
	for(i = 0; i < 500; i++) {
		tasks[i] = pool_task_create(pool, test_task, (void *)i);
		puts("done sleeping");
	}
	for(i = 0; i < 500; i++) {
		if(tasks[i] != NULL) 
		pool_join_task(tasks[i]);
		printf("task %p done\n", tasks[i]->result);
	}
	for(i = 0; i < 500; i++) {
		if(tasks[i] != NULL)
		pool_destroy_task(tasks[i]);
	}

	log_info("shutting down");
	pool_shutdown(pool);
	shutdown();
	return 0;
}

void shutdown(void) {
	log_info("Gracefully Shutting down SA Bot V3");
	log_puts("---------------------------------------------------------");
	log_end();
}

