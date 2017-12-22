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

static const char init_payload[] = {0x05, 0x01, 0x00};

static int try_connect(const char *server, int port);

int socks5_connect(const char *server, int port) {
	int fd, result;
	char buf[RECV_BUFFER_SIZE];

	fd = try_connect(server, port);
	if(fd < 0) {
		log_error("Connection to proxy server failed %s:%d", server, port);
		return fd;
	}
	send(fd, init_payload, sizeof(init_payload), 0);
	result = recv(fd, buf, 2, 0);
	if(result == 2)  {
		buf[2] = '\0';
		log_info("Got Server response %2x,%2x", *buf, *(buf + 1));
	}
	return fd;	
}


int try_connect(const char *server, int port) {
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

