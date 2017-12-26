#include "socks5.h"
#include "log.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define RECV_BUFFER_SIZE 512

static int proxy_try_connect(const char *server, int port);
static int proxy_subnegotiation(int fd);
static int proxy_request_details(int fd, const char *server, int port);

int socks5_connect(const char *proxy_server, int proxy_port,
		const char *server, int port) {
	int fd, result;

	fd = proxy_try_connect(proxy_server, proxy_port);
	if(fd < 0) {
		log_error("Connection to proxy server failed %s:%d", 
				proxy_server, proxy_port);
		return -1;
	}
    result = proxy_subnegotiation(fd);
    if(result < 0) {
        log_error("Subnegotaition failed");
        close(fd);
        return -1;
    }
    result = proxy_request_details(fd, server, port);
    
	return fd;	
}


int proxy_try_connect(const char *server, int port) {
	int fd, result;
	struct sockaddr_in dest;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0) {
		log_error_errno("Socket Error - socket value %d", fd);
		return fd;
	}
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	result = inet_aton(server, &dest.sin_addr);
	if(!result) {
		log_error_errno("Error on inet_aton() for server %s", server);
		close(fd);
		return -1;
	}
	result = connect(fd, (struct sockaddr *)&dest, sizeof(dest));
	if(result < 0) {
		log_error_errno("Error on connect() to server %s:%d", server, port);
		close(fd);
		return -1;
	}
	return fd;
}

int proxy_subnegotiation(int fd) {
	int result;
	char buf[RECV_BUFFER_SIZE];
    const char init_payload[] = 
        {0x05, 0x01, 0x00};

	send(fd, init_payload, sizeof(init_payload), 0);
	result = recv(fd, buf, 2, 0);
	if(result == 2)  {
		buf[2] = '\0';
		if(buf[0] == 0x5 && buf[1] == 0x0) {
			return 0;
		}
		else {
			log_error("Unexpected Server Response: %s", buf);
		}
	}
	else {
		log_error("Unexpected response size in connection");
    }
    return -1;
}

int proxy_request_details(int fd, const char *server, int port) {
    char payload[10];
    in_addr_t i_val;
    int result;
    char addr[4];
    char response[10];
	struct sockaddr_in dest;

    log_info("attempting to connect to server: %s:%d", server, port);
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	result = inet_aton(server, &dest.sin_addr);
	if(!result) {
		log_error_errno("Error on inet_aton() for server %s:%d", server, port);
		return -1;
	}
    payload[0] = 0x05;
    payload[1] = 0x01;
    payload[2] = 0x00;
    payload[3] = 0x01;

    i_val = dest.sin_addr.s_addr;
    memcpy(&payload[4], &i_val, sizeof(i_val));
    memcpy(&payload[8], &dest.sin_port, sizeof(dest.sin_port));

	send(fd, payload, sizeof(payload), 0);
	result = recv(fd, response, sizeof(response), 0);
    log_info("response size: %d\n", result);
    log_info("response: %d-%d", payload[0], payload[1]);
    return -1;
}

