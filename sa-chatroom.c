#include "log.h"
#include "sa-chatroom.h"
#include "net-general.h"
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

#define P_0 "0"
#define P_0C "0C"
#define P_01 "01"
#define P_03 "03_"
#define FIRST_JOIN_PACKET "02Z900_"

#define KEEP_ALIVE_INTERVAL 10

static int sa_login(sa_connection_s *con);
static void *sa_main_loop(void *args); 
static void *sa_keep_alive(void *args);

sa_connection_s *sa_create_connection(
        const char *server, int port,
        const char *username, const char *password
        ) {
    return sa_create_proxied_connection(
            NULL, 0, 
            server, port,
            username, password
            );
}

extern sa_connection_s *sa_create_proxied_connection(
        const char *proxy_server, int proxy_port,
        const char *server, int port, 
        const char *username, const char *password
        ) {
    sa_connection_s *con = sa_alloc(sizeof *con);
    con->state = SA_CON_INIT;
    con->proxy_server = proxy_server;
    con->proxy_port = proxy_port;
    con->server = server;
    con->port = port;
    con->fd = -1;
    strcpy(con->user.username, username);
    strcpy(con->password, password);
    map_init(&con->name_keyed_map);
    map_init(&con->id_keyed_map);
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
            log_error("sa login failed for user: %s", con->user.username);
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
    int result, size;
    const char init[] = PREFIX_KEY SERVER_KEY; 
    char buf[BUF_SIZE];

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
    result = recv(con->fd, buf, BUF_SIZE, 0);
    if(result != 3 || strcmp(buf, PREFIX_KEY)) {
        if(result == -1) {
            log_error_errno("Error on recv() after initial packet sent in %s()", 
                    __func__);
        }
        else {
            log_error("Error on recv() after initial packet sent in %s(), unexpected result: %s.", __func__, buf);
        }
        return -1;
    }
    size = sprintf(buf, PREFIX_LOGIN "%s;%s", con->user.username, con->password) +1; 
    result = send(con->fd, buf, size, 0);
    if(result != size) {
        if(result == -1) {
            log_info("buf result: %s", buf);
            log_error_errno("Error on send() for login credentials in %s()", __func__);
        }
        else {
            log_error("Error on send() for credentials in %s(). Unexpected number of bytes sent.", 
                    __func__);
        }
        return -1;
    }
    result = recv(con->fd, buf, BUF_SIZE, 0);
    if(result == -1) {
        log_error_errno("Error on recv() after sent credentials in %s()", __func__);
        return -1;
    }
    else if(result > 0) {
        log_info("result more than 0: %s", buf);

    }
    else {
        log_error("Failed to login users: %s", con->user.username);
        return -1;
    }
    strcpy(buf, P_0C);
    strcpy(buf + 3, P_01);
    result = send(con->fd, buf, 6, 0);
    if(result != 6) {
        if(result == -1) {
            log_error_errno("Error on send() after login in %s()", __func__);
        }
        else {
            log_error("Error on send() after login in %s(). Unexpected number of bytes sent.", 
                    __func__);
        }
        return -1;
    }
    result = recv(con->fd, buf, BUF_SIZE, 0);
    if(result == -1) {
        log_error_errno("Error on recv() 2 after credentials sent %s()", __func__);
        return -1;
    }
    if(buf[0] == '0' && buf[1] == '1' && !buf[2]) {
        result = send(con->fd, FIRST_JOIN_PACKET, sizeof(FIRST_JOIN_PACKET), 0);
        if(result != sizeof(FIRST_JOIN_PACKET)) {
            if(result == -1) {
                log_error_errno("Error on send() for FIRST_JOIN_PACKET in %s()", __func__);
            }
            else {
                log_error("Error on send() for FIRST_JOIN_PACKET in %s() - unexpected number of bytes sent.", __func__);
            }
            return -1;
        }
    }
    result = send(con->fd, P_03, sizeof(P_03), 0);
    if(result != sizeof(P_03)) {
        if(result == -1) {
            log_error_errno("Error on send() for final login packet in %s()", __func__);
        }
        else {
            log_error("Error on send() for final login packet in %s() - unexpected number of bytes sent.", __func__);
        }
        return -1;
    }

    result = pthread_create(&con->main_loop, NULL, sa_main_loop, con);
    if(result) {
        log_error_explicit(result, "Error on pthread_create() for main loop in %s()", __func__);
        return -1;
    }
    result = pthread_create(&con->keep_alive, NULL, sa_keep_alive, con);
    if(result) {
        log_error_explicit(result, "Error on pthread_create() for keep alive thread in %s()", __func__);
    }

    return 0;
}


void *sa_main_loop(void *args) {
    sa_connection_s *con = args;
    log_info("starting main loop for user: %s", con->user.username);
    pthread_exit(NULL);
}

void *sa_keep_alive(void *args) {
    sa_connection_s *con = args;
    int result, fd = con->fd;
    log_debug("starting keep alive thread for chatroom");

    while(con->state == SA_CON_CONNECTED) {
        result = sleep(KEEP_ALIVE_INTERVAL);
        if(!result) {
            result = send(fd, P_0, sizeof(P_0), 0);
            if(result != sizeof(P_0)) {
                if(result == -1) {
                    log_error_errno("Error while sending P_0 keep alive packet in %s().", __func__);
                }
                else {
                    log_error("Error while sending P_0 keep alive packet in %s() - Unexpected number of bytes sent.", __func__);
                }
            }
            result = send(fd, P_01, sizeof(P_01), 0);
            if(result != sizeof(P_01)) {
                if(result == -1) {
                    log_error_errno("Error while sending P_01 keep alive packet in %s().", __func__);
                }
                else {
                    log_error("Error while sending P_01 keep alive packet in %s() - Unexpected number of bytes sent.", __func__);
                }
            }
        }
    }
    pthread_exit(NULL);
}

