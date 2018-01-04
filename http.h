#ifndef __http_h__
#define __http_h__

#include "general.h"

#define MAX_METHOD_LEN 16
#define MAX_URI_LEN 2048
#define MAX_VERSION_LEN 16

#define HTTP_BAD_REQUEST -2
#define HTTP_UNSUPPORTED_METHOD -3

typedef struct http_request_s http_request_s;

struct http_request_s {
	char method[16];
	buf_s uri;
	char version[16];
	map_s headers;
	buf_s body;
};

extern int http_parse_request(http_request_s *req, char *raw);
extern void http_reqest_header_dealloc(http_request_s *req);
extern int http_do_request(http_request_s *req);

#endif

