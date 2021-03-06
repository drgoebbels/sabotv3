#include "http.h"
#include "db.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_URI_SCHEME 5
#define MAX_USERINFO 64
#define MAX_HOST 128
#define MAX_PORT 8
#define MAX_URI_PATH 128
#define URI_BUF_SIZE 512
#define URI_MAX_PORT 65535

typedef enum uri_tok_type_e uri_tok_type_e;

typedef struct uri_s uri_s;
typedef struct uri_tok_s uri_tok_s;
typedef struct uri_toklist_s uri_toklist_s;

enum uri_tok_type_e {
    URI_SEGMENT,
    URI_HIER,
    URI_AMP,
    URI_COLON,
    URI_END
};

struct uri_s {
    char scheme[MAX_URI_SCHEME];
    char user_name[MAX_USERINFO];
    char password[MAX_USERINFO];
    char host[MAX_HOST];
    bool is_ipv4;
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

static int http_syntax_scheme(uri_s *uri, uri_tok_s **tok);
static int http_syntax_user_info(uri_s *uri, uri_tok_s **tok);
static int http_syntax_host(uri_s *uri, uri_tok_s **tok);
static bool http_host_is_ipv4(char *lex);
static int http_syntax_port(uri_s *uri, uri_tok_s **tok);
static void http_free_toklist(uri_toklist_s *list);

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
        log_info("username: %s", u.user_name);
        log_info("password: %s", u.password);
        log_info("host: %s", u.host);
        log_info("is_ipv4: %d", u.is_ipv4);
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
    http_free_toklist(&list);
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
        else if(*fptr == '/' && *(fptr + 1) == '/') {
            http_add_token(list, "//", sizeof("//"), URI_HIER);
            fptr++;
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
    int result;
    uri_tok_s *tok = list->head;

    result = http_syntax_scheme(uri, &tok); 
    if(result == -1) {
        return -1;
    }
    if(tok->type != URI_HIER) {
        log_error("Syntax Error parsing URI in %s(): Expected '//' but got %s.", __func__, tok->lex);
        return -1;
    }
    tok = tok->next;
    result = http_syntax_user_info(uri, &tok);
    if(result == -1) {
        return -1;
    }

    result = http_syntax_host(uri, &tok);
    if(result == -1) {
        return -1;
    }
    result = http_syntax_port(uri, &tok);
    if(result == -1) {
        return -1;
    }
    if(tok->type == URI_SEGMENT) {
        if(tok->len < MAX_URI_PATH) {
            strcpy(uri->path, tok->lex);
            tok = tok->next;
        }
        else {
            log_error("Error parsing URI in %s(): URI path too long: %s.", __func__, tok->lex);
            return -1;
        }
    }
    if(tok->type != URI_END) {
        log_error("Syntax Error parsing URI in %s(): Unexpected token: %s.", __func__, tok->lex);
        return -1;
    }
    return 0;
    
}

int http_syntax_scheme(uri_s *uri, uri_tok_s **tok) {
    uri_tok_s *t = *tok, *scheme;
    if(t->type == URI_SEGMENT) {
        scheme = t;
        t = t->next; 
        if(t->type == URI_COLON) {
            if(!strcmp(scheme->lex, "http")) {
                strcpy(uri->scheme, "http");
                *tok = t->next;
                return 0;
            }
            else {
                log_error("Error parsing URI in %s(), unsupported scheme/protocol: %s. Only http supported.", __func__, scheme->lex);
            }
        }
        else {
            log_error("Syntax Error parsing URI in %s(), expected ':' but got %s.", __func__, t->lex);
        }
    }
    else {
        log_error("Syntax Error parsing URI in %s(), expected text sequence but got %s.", __func__, t->lex);
    }
    *tok = t;
    return -1;
}

/**
 * parsing userinfo: note, this is substandard
 */
int http_syntax_user_info(uri_s *uri, uri_tok_s **tok) {
    uri_tok_s *t = *tok, *user_name, *password;

    if(t->type == URI_SEGMENT) {
        user_name = t;
        t = t->next;
        if(t->type == URI_COLON) {
            t = t->next;
            if(t->type == URI_SEGMENT) {
                password = t;
                t = t->next;
                if(t->type == URI_AMP) {
                    *tok = t->next;
                    if(user_name->len >= MAX_USERINFO) {
                        log_error("Error parsing URI in %s() - username too long %s.", __func__, user_name->lex);
                        return -1;
                    }
                    else if(password->len >= MAX_USERINFO) {
                        log_error("Error parsing URI in %s() - password too long %s.", __func__, password->lex);
                        return -1;
                    }
                    else {
                        strcpy(uri->user_name, user_name->lex);
                        strcpy(uri->password, password->lex);
                        return 0;
                    }
                }
            }
        }
    }
    *uri->user_name = '\0';
    *uri->password = '\0';
    *tok = t;
    return 0;
}

int http_syntax_host(uri_s *uri, uri_tok_s **tok) {
    uri_tok_s *t = *tok;

    if(t->type == URI_SEGMENT) {
        if(t->len < MAX_HOST) {
            strcpy(uri->host, t->lex);
            uri->is_ipv4 = http_host_is_ipv4(t->lex);
            *tok = t->next;
            return 0;
        }
        else {
            log_error("Error parsing URI in %s(): Host name too long: %s", __func__, t->lex);
            *tok = t->next;
            return -1;
        }
    }
    else {
        log_error("Syntax Error parsing URI in %s(): Expected text sequence for host, but got %s.", __func__, t->lex);
        *tok = t->next;
        return -1;
    }
}

bool http_host_is_ipv4(char *lex) {
    int octet, i;
    char *bptr;
    
    for(i = 0; i < 4; i++) {
        if(isdigit(*lex)) {
            bptr = lex;
            while(isdigit(*++lex));
            if(*lex != '.') {
                if(*lex || (!*lex && i < 3))
                    return false;
            }
            *lex = '\0';
            octet = atoi(bptr);
            *lex = '.';
            if(octet > 255)
                return false;
            lex++;
        }
        else {
            return false;
        }
    }
    return true;
}

int http_syntax_port(uri_s *uri, uri_tok_s **tok) {
    int port;
    char *ptr, bck;
    uri_tok_s *t = *tok;
    
    if(t->type == URI_COLON) {
        t = t->next;
        if(t->type == URI_SEGMENT) {
            ptr = t->lex;
            while(*ptr) {
                if(!isdigit(*ptr)) {
                    if(*ptr == '/') {
                        break;
                    }
                    else {
                        log_error("Error parsing URI in %s() - invalid port number: %s.", __func__, t->lex);
                        *tok = t->next;
                        return -1;
                    }
                }
                ptr++;
            }
            bck = *ptr;
            *ptr = '\0';
            port = atoi(t->lex);
            *ptr = bck;
            if(port >= 0 && port <= URI_MAX_PORT) {
                uri->port = port;
                //transforming token (hacky solution to awkward parsing)
                t->len = strlen(ptr) + 1;
                memmove(t->lex, ptr, t->len);
                *tok = t;
                return 0;
            }
            else {
                log_error("Error parsing URI in %s() - port number out of range: %d", __func__, port);
                *tok = t->next;
                return -1;
            }
        }
        else {
            log_error("Syntax Error in URI in %s(): Expected digit sequence, but got %s.", __func__, t->lex);
            *tok = t->next;
            return -1;
        }
    }
    *tok = t->next;
    return 0;
}

void http_free_toklist(uri_toklist_s *list) {
    uri_tok_s *t = list->head, *bck;

    while(t) {
        bck = t->next;
        free(t->lex);
        free(t);
        t = bck;
    }
}

