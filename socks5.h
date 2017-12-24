#ifndef __socks5_h__
#define __socks5_h__

extern int socks5_connect(const char *proxy_server, int proxy_port,
			const char *server, int port);

#endif

