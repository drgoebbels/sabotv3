#include "general.h"
#include "server.h"
#include "log.h"



static void *server_thread(void *args);

int http_server_start(int port) {
	pthread_t pid;
	pthread_create(&pid, NULL, server_thread, NULL);
	log_info("Starting Server. Listening on Port: %d", port);
	return 0;
}

void *server_thread(void *args) {
	log_info("starting server");
	pthread_exit(NULL);	
}


