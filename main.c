#include "log.h"
#include "server.h"
#include <stdio.h>

#define LISTEN_PORT 5454

static void shutdown(void);

int main(void) {
	int result; 

	log_init_name("sabot.log");
	log_info("Starting SA Bot V3");
	result = server_start(LISTEN_PORT);
	if(result < 0) {
		log_error("Failed to start server on port %d.", LISTEN_PORT);
	}
	return 0;
}

void shutdown(void) {
	log_info("Gracefully Shutting down SA Bot V3");
	log_end();
}

