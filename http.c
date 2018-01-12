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

typedef struct uri_s uri_s;

struct uri_s {
	char scheme[MAX_URI_SCHEME];
	char user_info[MAX_USERINFO]; 
	char host[MAX_HOST];
	int port;
    char path[MAX_URI_PATH];
};

static const char *supported_methods[] = {
	"GET", "POST"
};

static int http_parse_request_line(http_request_s *req, char **ptr);
static bool http_check_method(const char *method);

static int http_resolve_address(const char *host_name, char *address_buffer);
static int http_parse_uri(uri_s *uri, const char *raw);

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

int http_parse_uri(uri_s *uri, const char *raw) {
	int i, j;
    size_t len;
	char *scheme = uri->scheme;
    char *uri_ptr = uri->path;
	char buf[MAX_PORT], bck;
    char uri_buf[URI_BUF_SIZE];
	char *ptr = uri_buf, *bptr;

    len = strlen(raw);
    if(len >= URI_BUF_SIZE) {
        log_error("Error parsing URI in %s() - URI too long %s.", __func__, uri_buf);
        return -1;
    }
    strcpy(uri_buf, raw);

	if(!isalpha(*ptr)) {
		log_error("Error in %s() parsing URI %s: invalid URI scheme", __func__, uri_buf);
		return -1;
	}
	i = 1;
	scheme[0] = *ptr++;
	while(isalnum(*ptr) || *ptr == '+' || *ptr == '-' || *ptr == '.') {
		scheme[i++] = *ptr++;
		if(i == MAX_URI_SCHEME) {
			log_error("Error in %s() parsing URI %s: URI scheme too long", __func__, uri_buf);
			return -1;
		}
	}
	scheme[i] = '\0';
	if(strcmp(scheme, "http")) {
		log_error("Error in %s() - Protocol/scheme %s not supported. Only http supported.", __func__, scheme);
		return -1;
	}
	if(*ptr != ':') {
		log_error("Error in %s() parsing URI %s: Expected ':' after scheme, but got %c.", __func__, uri_buf, *ptr);
		return -1;
	}
	ptr++;
	if(*ptr != '/' || *(ptr + 1) != '/') {
		log_error("Error in %s() parsing URI %s: Expected '//' after scheme, but got %c%c", __func__, uri_buf, *ptr, *(ptr + 1));
		return -1;
	}
	ptr += 2;
	/*
	 * authority   = [ userinfo "@" ] host [ ":" port ]
	 *
	 * userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
	 * unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
	 * pct-encoded = "%" HEXDIG HEXDIG
	 * sub-delims  = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="	
	 */
	i = 0;
	bptr = ptr;
	*uri->user_info = '\0';
	while(*ptr) {
		if(*ptr == '@') {
			if(i >= MAX_USERINFO) {
				log_error("Error in %s() parsing URI %s: URI user info too long.", __func__, uri_buf);
				return -1;
			}
			*ptr = '\0';
			strcpy(uri->user_info, bptr);
			bptr = ptr + 1;
			*ptr = '@';
		}
        else if(*ptr == ':') {
            if(i < MAX_HOST) {
                *ptr = '\0';
                strcpy(uri->host, bptr);
                *ptr++ = ':';
            }
            else {
                log_error("Error parsing host name in %s(). URI %s too long.", __func__, bptr);
                return -1;
            }
            for(j = 0; j < MAX_PORT; j++) {
                if(*ptr == '/' || !*ptr) {
                    bck = *ptr;
                    *ptr = '\0';
                    uri->port = atoi(bptr);
                    *ptr = bck;
                    break;
                }
                else if(isdigit(*ptr)) {
                    ptr++;
                }
                else {
                    log_error("Failed to parse port in %s() at ptr: %s.", __func__, bptr);
                    return -1;
                }
            }
        }
		else if(*ptr == '/') {
		    if(i < MAX_HOST) {
                *ptr = '\0';
                strcpy(uri->host, bptr);
                *ptr = '/';
            }
            else {
                log_error("Failed to parse host in %s(), host name too long for %s.", __func__, uri_buf);
                return -1;
            }
		}
		i++;
	}
    if(*ptr == '/') {
        for(i = 0; *ptr && i < MAX_URI_PATH; i++) {
            uri_ptr[i] = *ptr++;
        }
        if(i == MAX_URI_PATH) {
            log_error("Failed to parse URI path in %s(), path too long: %s.", __func__, uri_buf);
            return -1;
        }
        uri_ptr[i] = '\0';
    }
    return 0;
}

