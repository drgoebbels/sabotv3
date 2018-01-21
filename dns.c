#include "dns.h"
#include "log.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



int dns_resolve(const char *address) {
	int result;
	char host_buf[256];
	struct addrinfo hints, *addresses, *ptr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	result = getaddrinfo(address, NULL, &hints, &addresses);
	if(result) {
		log_error("Error in %s() - getaddrinfo() call on address %s failed: %s", __func__, address, gai_strerror(result));
		return -1;
	}

	for(ptr = addresses; ptr; ptr = ptr->ai_next) {
		getnameinfo(ptr->ai_addr, ptr->ai_addrlen, host_buf, sizeof(host_buf), NULL, 0, NI_NUMERICHOST);
		log_info("host: %s", host_buf);
	}

	freeaddrinfo(addresses);
	return 0;
}


