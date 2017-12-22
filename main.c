#include "log.h"
#include "server.h"
#include "socks5.h"
#include <stdio.h>

#define LISTEN_PORT 5454
#define PROXY_SERVER "127.0.0.1"
#define PROXY_PORT 9050

static void shutdown(void);

int main(void) {
	int result, fd; 

	log_init_name("sabot.log");
	log_puts("---------------------------------------------------------");
	log_info("Starting SA Bot V3");
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
	log_info("shutting down");
	shutdown();
	return 0;
}

void shutdown(void) {
	log_info("Gracefully Shutting down SA Bot V3");
	log_puts("---------------------------------------------------------");
	log_end();
}

