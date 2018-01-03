#include "http.h"
#include "log.h"
#include <string.h>
#include <stdbool.h>

static const char *supported_methods[] = {
	"GET", "POST"
};

static int http_parse_request_line(http_request_s *req, char **ptr);
static bool http_check_method(const char *method);

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

