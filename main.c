#include "log.h"
#include "db.h"
#include "pool.h"
#include "http.h"
#include "sa-chatroom.h"
#include "sa-map.h"
#include "server.h"
#include "socks5.h"
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#define LISTEN_PORT 5454
#define PROXY_SERVER "127.0.0.1"
#define PROXY_PORT 9050
#define PASSWORD_FILE ".pwd"
#define DATABASE_NAME "db/sa.db"

static int sa_login_from_file_tor(const char *server);
static void shutdown(void);

static void test_map_file(const char *name);

int main(void) {
    int result; 
	http_server_s http_server;

    log_init_name("sabot.log");
    log_puts("##########################################################");
    log_puts("------------------- Starting SA Bot V3 -------------------");
    log_puts("==========================================================");
    db_init(DATABASE_NAME);
    result = http_server_start(&http_server, LISTEN_PORT);
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
	pthread_join(http_server.pid, NULL);
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
            continue;
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
	test_map_file("xgen-data/maps/__xgenhq.dat");
    con = sa_create_proxied_connection(
            PROXY_SERVER, PROXY_PORT,
            server, SA_PORT,
            username, password
            );
    result = sa_connect(con);
	if(result) {
		return -1;
	}
    pthread_join(con->main_loop, NULL);
    return result;
}

void shutdown(void) {
    log_info("Gracefully Shutting down SA Bot V3");
    log_end();
}

void test_map_file(const char *name) {
	int result;
	sa_map_s map;
	buf_s src = read_file(name);

	result = sa_parse_map(&map, src);

}

