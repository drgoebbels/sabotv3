#include "log.h"
#include "pool.h"
#include "sa-chatroom.h"
#include "server.h"
#include "socks5.h"
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#define LISTEN_PORT 5454
#define PROXY_SERVER "127.0.0.1"
#define PROXY_PORT 9050
#define PASSWORD_FILE ".pwd"

static int sa_login_from_file_tor(const char *server);
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
	int result; 

	log_init_name("sabot.log");
	log_puts("##########################################################");
	log_puts("------------------- Starting SA Bot V3 -------------------");
	log_puts("==========================================================");
	result = server_start(LISTEN_PORT);
	if(result < 0) {
		log_error("Failed to start server on port %d.", LISTEN_PORT);
	}
    result = sa_login_from_file_tor(SERVER_2D_CENTRAL);
	if(result == 0) {
		log_info("Successful connection");
	}
	else {
		log_error("Unsuccessful connection");
	}
	shutdown();
	return 0;
}

int sa_login_from_file_tor(const char *server) {
    int c, i = 0, result;
    sa_connection_s *con;
    FILE *f;
    char username[MAX_USERNAME_SIZE + 1];
    char password[MAX_PASSWORD_SIZE + 1];
    char *ptr = username;
    
    f = fopen(PASSWORD_FILE, "r");
    if(!f) {
        log_error_errno("Failed to open password file: %s in %s()", 
                PASSWORD_FILE, __func__);
        return -1;
    }
    while((c = fgetc(f)) != EOF) {
        if(c == ':') {
            *ptr = '\0';
            ptr = password;
            i = 0;
        }
        else if(isspace(c)) {
            ptr++;
        }
        else if(i == MAX_USERNAME_SIZE) {
           log_error("Error reading password file in %s(): username or password too long.", __func__);
            fclose(f);
            return -1;
        }
        else {
            *ptr++ = c;
            i++;
        }
    }
    fclose(f);
    *ptr = '\0';
    if(!*username || !*password) {
        log_error("Error Reading password file in %s(): Parsing failed, invalid format.", __func__);
        return -1;
    }
    con = sa_create_proxied_connection(
            PROXY_SERVER, PROXY_PORT,
            server, SA_PORT,
            username, password
            );
    result = sa_connect(con);
    return result;
}

void shutdown(void) {
	log_info("Gracefully Shutting down SA Bot V3");
	log_end();
}

