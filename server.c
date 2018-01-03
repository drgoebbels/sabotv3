#include "general.h"
#include "server.h"
#include "log.h"
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_BUF_SIZE 1024
#define SERVER_POOL_SIZE 16
#define SERVER_MAX_BACKLOG 128

typedef struct sa_task_s sa_task_s;

struct sa_task_s {
	int fd;
	struct sockaddr_in ip;
};

static void *server_thread(void *args);
static void *sa_taskf(void *args);

int http_server_start(http_server_s *server, int port) {
	int result;
	server->port = port;
	server->pool = pool_init(SERVER_POOL_SIZE);
	if(!server->pool) 
		return -1;
	result = pthread_create(&server->pid, NULL, server_thread, server);
	if(result) {
		log_error_errno("Error in %s() - Failed to start server thread.");
		pool_shutdown(server->pool);
		return -1;
	}
	return 0;
}

void *server_thread(void *args) {
	int fd, client_fd, result;
	socklen_t address_size;
	sa_task_s *task_arg;
	pool_task_s *sa_task;
	char buf[SERVER_BUF_SIZE];
	struct sockaddr_in server_address, 
					   client_address;
	http_server_s *server = args;
	pool_s *pool = server->pool;

	log_info("starting server on: %d", server->port);
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		log_error_errno("Error in %s() - Failed on socket()", __func__);
		pthread_exit(NULL);
	}
	memset(&server_address, 0, sizeof(server_address));
	memset(&server_address, 0, sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(server->port);
	
	result = bind(fd, (struct sockaddr *)&server_address, sizeof(server_address));
	if(result == -1) {
		log_error_errno("Error in %s() - Failed on bind() for port: %d", __func__, server->port);
		close(fd);
		pthread_exit(NULL);
	}
	result = listen(fd, SERVER_MAX_BACKLOG);
	if(result == -1) {
		log_error_errno("Error in %s() - Failed on listen() for port: %d", __func__, server->port);
		close(fd);
		pthread_exit(NULL);
	}
	address_size = sizeof(client_address);
	while(true) {
		client_fd = accept(fd, (struct sockaddr *) &client_address, &address_size);
		log_info("accepted");
		if(client_fd == -1) {
			log_error_errno("Error during accept in %s(), continuing anyway", __func__);
		}
		else {
			task_arg = sa_alloc(sizeof *task_arg);
			task_arg->fd = client_fd;
			memcpy(&task_arg->ip, &client_address, sizeof(client_address));
			pool_task_create(pool, sa_taskf, task_arg, true);
		}
	}
	pthread_exit(NULL);	
}

void *sa_taskf(void *args) {
	ssize_t result;
	sa_task_s *task = args;
	char buf[SERVER_BUF_SIZE + 1];
	log_info("calling sa_task() with fd: %d", task->fd);
	
	while(read(task->fd, buf, SERVER_BUF_SIZE) > 0) {
		buf[SERVER_BUF_SIZE] = '\0';
		write(task->fd, "derp", sizeof("derp"));
		log_info("read: %s", buf);
	}
	close(task->fd);
	free(task);
	return NULL;
}

