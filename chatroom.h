#ifndef __chatroom_h__
#define __chatroom_h__

#define MAX_USERNAME_SIZE 20
#define MAX_PASSWORD_SIZE 20

typedef struct sa_connection_s sa_connection_s;

struct sa_connection_s {

	char username[MAX_USERNAME_SIZE];
	char password[MAX_PASSWORD_SIZE];
};

const char *SERVER_2D_CENTRAL = "45.76.234.65";
//more servers here

//extern void 

#endif
