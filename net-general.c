#include "log.h"
#include "net-general.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int net_try_connect(const char *server, int port) {
	int fd, result;
	struct sockaddr_in dest;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		log_error_errno("Socket Error in %s() - socket value %d", __func__, fd);
		return fd;
	}
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	result = inet_aton(server, &dest.sin_addr);
	if(!result) {
		log_error_errno("Error on inet_aton() in %s() for server %s", __func__, server);
		close(fd);
		return -1;
	}
	result = connect(fd, (struct sockaddr *)&dest, sizeof(dest));
	if(result == -1) {
		log_error_errno("Error on connect() in %s() to server %s:%d", __func__, server, port);
		close(fd);
		return -1;
	}
	return fd;
}

int net_check_result(const char *funcc, int result, int expected) {
    if(result < 0) {
    }
    else {
    }
}

