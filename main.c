#include "log.h"
#include "pool.h"
#include "chatroom.h"
#include "server.h"
#include "socks5.h"
#include <stdio.h>
#include <unistd.h>

#define LISTEN_PORT 5454
#define PROXY_SERVER "127.0.0.1"
#define PROXY_PORT 9050

static void shutdown(void);

void *test_task(void *arg) {
	log_info("hello from %p", arg);
	return arg;
}

static void thread_pool_test(void) {
	pool_s *pool;
	pool_task_s *tasks[500];
	pool = pool_init(10);
	if(!pool) {
		log_error("Failed to start thread pool.");
	}
	log_info("task testing");
	long i;
	for(i = 0; i < 500; i++) {
		tasks[i] = pool_task_create(pool, test_task, (void *)i);
	}
	for(i = 0; i < 500; i++) {
		if(tasks[i] != NULL) 
		pool_join_task(tasks[i]);
	}
	for(i = 0; i < 500; i++) {
		if(tasks[i] != NULL)
		pool_destroy_task(tasks[i]);
	}
	log_info("shutting down");
	pool_shutdown(pool);
}

int main(void) {
	int result, fd; 

	log_init_name("sabot.log");
	log_puts("##########################################################");
	log_puts("------------------- Starting SA Bot V3 -------------------");
	log_puts("==========================================================");
	result = server_start(LISTEN_PORT);
	if(result < 0) {
		log_error("Failed to start server on port %d.", LISTEN_PORT);
	}
	fd = socks5_connect(PROXY_SERVER, PROXY_PORT, SERVER_2D_CENTRAL, SA_PORT + 1);
	if(fd >= 0) {
		log_info("Successful connection: Got fd: %d", fd);
	}
	else {
		log_error("Unsuccessful connection: Got fd: %d", fd);
	}
	
	shutdown();
	return 0;
}

void shutdown(void) {
	log_info("Gracefully Shutting down SA Bot V3");
	log_end();
}

