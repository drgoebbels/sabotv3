#ifndef __server_h__
#define __server_h__

#include <pthread.h>

typedef struct http_server_s http_server_s;

extern int http_server_start(int port);

#endif

