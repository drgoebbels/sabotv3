#ifndef __net_general_h__
#define __net_general_h__

extern int net_try_connect(const char *server, int port);
extern int net_check_result(const char *funcc, int result, int expected);

#endif

