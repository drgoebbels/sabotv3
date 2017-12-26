#ifndef __socks5_h__
#define __socks5_h__

typedef enum socks5_method_e socks5_method_e;

enum socks5_method_e {
    SOCKS5_NOAUTH,
    SOCKS5_GSSAPI,
    SOCKS5_UNAMEPASS,
    SOCKS5_IANA, //0X03..0X7F
    SOCKS5_RESERVED = 0X80, //0X80..0XFE
    SOCKS5_NOMETHODS = 0XFF
};

extern int socks5_connect(const char *proxy_server, int proxy_port,
			const char *server, int port);

#endif

