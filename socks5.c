#include "socks5.h"
#include "log.h"
#include "net-general.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define RECV_BUFFER_SIZE 512

static int proxy_pick_method(int fd, socks5_method_e method);
static int proxy_connect_ipv4(int fd, const char *server, int port);

int socks5_connect(const char *proxy_server, int proxy_port,
		const char *server, int port) {
	int fd, result;

	fd = net_try_connect(proxy_server, proxy_port);
	if(fd < 0) {
		log_error("Connection to proxy server failed %s:%d", 
				proxy_server, proxy_port);
		return -1;
	}
	result = proxy_pick_method(fd, SOCKS5_NOAUTH);
	if(result < 0) {
		log_error("Subnegotaition failed");
		close(fd);
		return -1;
	}
	result = proxy_connect_ipv4(fd, server, port); 
	return fd;	
}


int proxy_pick_method(int fd, socks5_method_e method) {
	int result;
	char buf[2];
	const char init_payload[] = 
	{0x05, 0x01, method};

	result = send(fd, init_payload, sizeof(init_payload), 0);
	if(result != sizeof(init_payload)) {
		if(result == -1) {
			log_error_errno("An error occurred on send() in %s().", __func__);
		}
		else {
			log_error("Unexpected number of bytes sent to server in %s(): %d", __func__, result);
		}
		return -1;
	}
	result = recv(fd, buf, sizeof(buf), 0);
	if(result == sizeof(buf))  {
		if(buf[0] == 0x05 && buf[1] == 0x00) {
			return 0;
		}
		else {
			log_error("Unexpected Server Response in %s(): 0x%02x.0x%02x", buf[0], buf[1]);
		}
	}
	else {
		if(result == -1) {
			log_error_errno("An error occurred on recv() in %s()", __func__);
		}
		else {
			log_error("Unexpected number of bytes received from server in %s(): %d", __func__, result);
		}
	}
	return -1;
}

int proxy_connect_ipv4(int fd, const char *server, int port) {
	char buf[10];
	in_addr_t i_val;
	int result;
	struct sockaddr_in dest;

	log_info("attempting to connect to server: %s:%d", server, port);
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	result = inet_aton(server, &dest.sin_addr);
	if(!result) {
		log_error_errno("Error on inet_aton() for server %s:%d in %s()", server, port, __func__);
		return -1;
	}
	buf[0] = 0x05;
	buf[1] = 0x01;
	buf[2] = 0x00;
	buf[3] = 0x01;

	i_val = dest.sin_addr.s_addr;
	memcpy(&buf[4], &i_val, sizeof(i_val));
	memcpy(&buf[8], &dest.sin_port, sizeof(dest.sin_port));

	result = send(fd, buf, sizeof(buf), 0);
	if(result != sizeof(buf)) {
		if(result == -1) {
			log_error_errno("An error occurred on send() in %s().", __func__);
		}
		else {
			log_error("Unexpected number of bytes sent to server in %s(): %d", __func__, result);
		}
		return -1;
	}
	result = recv(fd, buf, sizeof(buf), 0);
	if(result == sizeof(buf)) {
		if(buf[0] == 0x05 && buf [1] == 0x00) {
			return 0;
		}
		else {
			log_error("Unexpected Server Response in %s(): 0x%02x.0x%02x", 
					__func__, buf[0], buf[1]);
		}
	}
	else {
		if(result == -1) {
			log_error_errno("An error occurred on recv() in %s()", __func__);
		}
		else {
			log_error("Unexpected number of bytes received from server in %s(): %d", __func__, result);
		}
	}
	return -1;
}

