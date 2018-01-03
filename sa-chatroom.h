#ifndef __sa_chatroom_h__
#define __sa_chatroom_h__

#include "general.h"
#include <pthread.h>

#define MAX_USERNAME_SIZE 20
#define MAX_PASSWORD_SIZE 20

#define SA_PORT 1138

//ballistick5.xgenstudios.com
#define SERVER_2D_CENTRAL "45.76.234.65"

typedef enum sa_con_state_e sa_con_state_e;
typedef enum sa_arena_e sa_arena_e;
typedef enum sa_arena_type_e sa_arena_type_e;

typedef struct sa_user_s sa_user_s;
typedef struct sa_connection_s sa_connection_s;
typedef struct sa_vector_s sa_vector_s;

enum sa_con_state_e {
    SA_CON_INIT,
    SA_CON_CONNECTED,
    SA_CON_SHUTDOWN,
    SA_CON_DISCONNECTED,
};

enum sa_arena_e {
    SA_ARENA_XGENHQ
};

enum sa_arena_type_e {
    SA_ARENA_TYPE_CYCLE,
    SA_ARENA_TYPE_RANDOM,
    SA_ARENA_TYPE_REPEAT
};

struct sa_user_s {
    char id[4];
    char username[MAX_USERNAME_SIZE + 1];
    struct {
        char r, g, b;
    } color;
};

struct sa_connection_s {
    sa_con_state_e state;
    const char *proxy_server;
    int proxy_port;
    const char *server;
    int port;
    int fd;
	char password[MAX_PASSWORD_SIZE + 1];
    sa_user_s user;
    map_s id_keyed_map;
    map_s name_keyed_map;
    pthread_t main_loop;
    pthread_t keep_alive;
};

struct sa_vector_s {
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

