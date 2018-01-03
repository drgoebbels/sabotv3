#include "general.h"
#include "server.h"
#include "log.h"
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_BUF_SIZE 1024
#define SERVER_POOL_SIZE 16

static void *server_thread(void *args);

int http_server_start(http_server_s *server, int port) {
	int result;
	server->port = port;
	result = pthread_create(&server->pid, NULL, server_thread, server);
	if(result) {
		log_error_errno("Error in %s() - Failed to start server thread.");
		return -1;
	}
	server->pool = pool_init(SERVER_POOL_SIZE);
	if(!server->pool) 
		return -1;
	return 0;
}

void *server_thread(void *args) {
	char buf[SERVER_BUF_SIZE];
	struct sockaddr_in address;
	http_server_s *server = args;

	log_info("starting server on: %d", server->port);

	pthread_exit(NULL);	
}


