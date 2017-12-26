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
    if(con->proxy_server) {
        con->fd = socks5_connect(con->proxy_server, con->proxy_port, 
                con->server, con->port);
    }
    else {
        con->fd = net_try_connect(con->server, con->port);
    }
    if(con->fd >= 0) {
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

