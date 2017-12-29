#include "log.h"
#include "sa-chatroom.h"
#include "net-general.h"
#include "general.h"
#include "socks5.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 256

#define SERVER_KEY "HxO9TdCC62Nwln1P"
#define PREFIX_KEY "08"
#define PREFIX_LOGIN "09"

static int sa_login(sa_connection_s *con);

sa_connection_s *sa_create_connection(
        const char *server, int port,
        const char *username, const char *password
        ) {
    sa_connection_s *con = sa_alloc(sizeof *con);
    con->proxy_server = NULL;
    con->proxy_port = 0;
    con->server = server;
    con->port = port;
    con->fd = -1;
    strcpy(con->username, username);
    strcpy(con->password, password);
    return con;
}

extern sa_connection_s *sa_create_proxied_connection(
        const char *proxy_server, int proxy_port,
        const char *server, int port, 
        const char *username, const char *password
        ) {
    sa_connection_s *con = sa_alloc(sizeof *con);
    con->proxy_server = proxy_server;
    con->proxy_port = proxy_port;
    con->server = server;
    con->port = port;
    con->fd = -1;
    strcpy(con->username, username);
    strcpy(con->password, password);
    return con;
};

int sa_connect(sa_connection_s *con) {
	int result;

    if(con->proxy_server) {
        con->fd = socks5_connect(con->proxy_server, con->proxy_port, 
                con->server, con->port);
    }
    else {
        con->fd = net_try_connect(con->server, con->port);
    }
    if(con->fd >= 0) {
		result = sa_login(con);
		if(result) {
			log_error("sa login failed for user: %s", con->username);
		}
        return 0;
    }
    return -1;
}

int sa_direct_connect(sa_connection_s *con) {
    int fd;
    struct sockaddr_in dest;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        log_error_errno("Error on socket() in %s()", __func__);
        return -1;
    }

}

int sa_login(sa_connection_s *con) {
    int result;
    const char init[] = PREFIX_KEY SERVER_KEY; 

	log_debug("Sending server key");
    result = send(con->fd, init, sizeof(init), 0);
    if(result != sizeof(init)) {
		if(result == -1) {
			log_error_errno("Error on send() for initial server key in %s()", 
				__func__);	
		}
		else {
			log_error("Error on send() for inital server key in %s(). Unexpected number of bytes sent.", 
					__func__);
		}
		return -1;
    }
	return 0;
}

