#ifndef __server_h__
#define __server_h__

#include "pool.h"
#include <pthread.h>

typedef struct http_server_s http_server_s;

struct http_server_s {
	int fd;
	int port;
	pthread_t pid;
	pool_s *pool;
};

extern int http_server_start(http_server_s *server, int port);

#endif

