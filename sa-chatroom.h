#ifndef __sa_chatroom_h__
#define __sa_chatroom_h__

#define MAX_USERNAME_SIZE 20
#define MAX_PASSWORD_SIZE 20

#define SA_PORT 1138

#define SERVER_2D_CENTRAL "45.76.234.65"

typedef struct sa_connection_s sa_connection_s;

struct sa_connection_s {
    const char *proxy_server;
    int proxy_port;
    const char *server;
    int port;
    int fd;
	char username[MAX_USERNAME_SIZE + 1];
	char password[MAX_PASSWORD_SIZE + 1];
};

extern sa_connection_s *sa_create_connection(
        const char *server, int port,
        const char *username, const char *password
        );

extern sa_connection_s *sa_create_proxied_connection(
        const char *proxy_server, int proxy_port,
        const char *server, int port, 
        const char *username, const char *password
        );

extern int sa_connect(sa_connection_s *con);


#endif

