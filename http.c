#include "http.h"
#include "db.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_URI_SCHEME 32
#define MAX_USERINFO 64
#define MAX_HOST 128
#define MAX_PORT 8
#define MAX_URI_PATH 128
#define URI_BUF_SIZE 512

typedef enum uri_tok_type_e uri_tok_type_e;

typedef struct uri_s uri_s;
typedef struct uri_tok_s uri_tok_s;
typedef struct uri_toklist_s uri_toklist_s;

enum uri_tok_type_e {
    URI_SEGMENT,
    URI_AMP,
    URI_COLON,
    URI_END
};

struct uri_s {
	char scheme[MAX_URI_SCHEME];
	char user_info[MAX_USERINFO]; 
	char host[MAX_HOST];
	int port;
    char path[MAX_URI_PATH];
};

struct uri_tok_s {
    uri_tok_type_e type;
    size_t len;
    char *lex;
    uri_tok_s *prev, *next;
};

struct uri_toklist_s {
    uri_tok_s *head;
    uri_tok_s *tail;
};

static const char *supported_methods[] = {
	"GET", "POST"
};

static int http_parse_request_line(http_request_s *req, char **ptr);
static bool http_check_method(const char *method);

static int http_resolve_address(const char *host_name, char *address_buffer);
static int http_parse_uri(uri_s *uri, const char *raw);
static int http_lex_uri(uri_toklist_s *list, const char *raw);
static void http_add_token(uri_toklist_s *list, char *lex, size_t lex_len, uri_tok_type_e type);
static int http_syntax_uri(uri_s *uri, uri_toklist_s *list);

void http_test(char *uri) {
    int result;
    uri_s u;

	log_info("parsing uri: %s", uri);
    result = http_parse_uri(&u, uri);
    if(result) {
        log_error("parse test failed");
    }
    else {
        log_info("parse test passed");
        log_info("scheme: %s", u.scheme);
        log_info("user_info: %s", u.user_info);
        log_info("host: %s", u.host);
        log_info("port: %d", u.port);
        log_info("path: %s", u.path);
    }
}

int http_parse_request(http_request_s *req, char *raw) {
	int result;

	result = http_parse_request_line(req, &raw);
	if(result) 
		return -1;
	log_info("method: %s uri: %s version: %s", req->method, req->uri, req->version);
	return 0;
}

int http_parse_request_line(http_request_s *req, char **ptr) {
	int i;
	buf_s *uri = &req->uri;
	char *fptr = *ptr;
	char *method = req->method;
	char *version = req->version;
		
	for(i = 0; i < MAX_METHOD_LEN; i++) {
		if(*fptr == ' ') {
			method[i] = '\0';
			if(!http_check_method(method)) {
				log_error("HTTP Unsupported Request method: %s", method);
				return HTTP_UNSUPPORTED_METHOD;
			}
			fptr++;
			break;
		}
		method[i] = *fptr++;
	}
	if(i == MAX_METHOD_LEN) {
		method[i - 1] = '\0';
		log_error("Request Method too long: %s", method);
		return HTTP_BAD_REQUEST;
	}
	buf_init(uri);
	for(i = 0; i < MAX_URI_LEN; i++) {
		if(*fptr == ' ') {
			buf_add_char(uri, '\0');
			fptr++;
			break;
		}
		buf_add_char(uri, *fptr);
		fptr++;
	}
	if(i == MAX_URI_LEN) {
		buf_add_char(uri, '\0');
		log_error("URI too long: %s", uri->data);
		return HTTP_BAD_REQUEST;
	}
	for(i = 0; i < MAX_VERSION_LEN; i++) {
		if(*fptr == '\r') {
			if(*(fptr + 1) == '\n') {
				version[i] = '\0';
				fptr += 2;
				break;
			}
			else {
				log_error("Error parsing http header: cr not followed by lf for uri: %s", uri->data);
				buf_dealloc(uri);
				return HTTP_BAD_REQUEST;
			}
		}
		version[i] = *fptr++;
	}
	if(i == MAX_VERSION_LEN) {
		version[i - 1] = '\0';
		log_error("Error parsing http header: version too long for uri: %s - version: %s", uri->data, version);
		buf_dealloc(uri);
		return HTTP_BAD_REQUEST;
	}
	*ptr = fptr;
	return 0;
}

bool http_check_method(const char *method) {
	size_t i, len;

	len = sizeof(supported_methods)/sizeof(char *);
	for(i = 0; i < len; i++) {
		if(!strcmp(supported_methods[i], method))
			return true;
	}
	return false;
}

void http_reqest_header_dealloc(http_request_s *req) {
	buf_dealloc(&req->uri);
	buf_dealloc(&req->uri);
	map_dealloc(&req->headers);
}

int http_do_request(http_request_s *req) {
	uri_s uri;
	
}


int http_resolve_address(const char *host_name, char *address_buffer) {
	int result;
	
	result = db_check_dns_cache(host_name, address_buffer);

	return 0;
}

/*
 * authority   = [ userinfo "@" ] host [ ":" port ]
 *
 * userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
 * unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
 * pct-encoded = "%" HEXDIG HEXDIG
 * sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="	
 */
int http_parse_uri(uri_s *uri, const char *raw) {
    int result;
    uri_tok_s *tok;
    uri_toklist_s list = {NULL, NULL};    
    
    result = http_lex_uri(&list, raw);
    if(result == -1) {
        log_error("Lexical Analysis on URI %s failed in %s()", raw, __func__);
        return -1;
    }
    result = http_syntax_uri(uri, &list);
    if(result == -1) {
        log_error("Syntax Analysis on URI %s failed in %s()", raw, __func__);
        return -1;
    }
    return 0;
}

int http_lex_uri(uri_toklist_s *list, const char *raw) {
    size_t len;
    char buf[URI_BUF_SIZE];
    char *fptr = buf, *bptr = buf;

    len = strlen(raw);
    if(len >= URI_BUF_SIZE) {
        log_error("Error in %s(), URI %s too long for parsing.", __func__, raw);
        return -1;
    }
    strcpy(buf, raw);

    while(*fptr) {
        if(*fptr == '@') {
            if(fptr - 1 > buf) {
                *fptr = '\0';
                http_add_token(list, bptr, fptr - bptr + 1, URI_SEGMENT);
            }
            http_add_token(list, "@", sizeof("@"), URI_AMP); 
            bptr = fptr + 1;
        }
        else if(*fptr == ':') {
            if(fptr - 1 > buf) {
                *fptr = '\0';
                http_add_token(list, bptr, fptr - bptr + 1, URI_SEGMENT);
            }
            http_add_token(list, ":", sizeof(":"), URI_COLON); 
            bptr = fptr + 1;
        }
        fptr++;
    }
    if(*bptr) {
        http_add_token(list, bptr, fptr - bptr + 1, URI_SEGMENT);
    }
    http_add_token(list, "@EOF@", sizeof("@EOF@"), URI_END);
    return 0;
}

void http_add_token(uri_toklist_s *list, char *lex, size_t lex_len, uri_tok_type_e type) {
    uri_tok_s *t = sa_alloc(sizeof *t);
    t->lex = sa_alloc(lex_len);
    t->type = type;
    t->next = NULL;
    strcpy(t->lex, lex);
    if(list->head) {
        t->prev = list->tail;
        list->tail->next = t;
    }
    else {
        list->head = t;
        t->prev = NULL;
    }
    list->tail = t;
}

int http_syntax_uri(uri_s *uri, uri_toklist_s *list) {
    
}

