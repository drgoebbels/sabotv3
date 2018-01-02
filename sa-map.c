#include "sa-map.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LEX_LEN 128

typedef enum map_token_e map_token_e;

typedef struct map_token_s map_token_s;
typedef struct map_tokens_s map_tokens_s;

enum map_token_e {
	MAP_TOK_IDENT,
	MAP_TOK_INT,
	MAP_TOK_AMP,
	MAP_TOK_EQ,
	MAP_TOK_EOF
};

struct map_token_s {
	map_token_e type;
	char lex[MAX_LEX_LEN];
};

static void map_lex(char *src);
static map_token_s *map_next_tok(char **ptr);
static map_token_s *map_token_init(char *lexeme, int len, map_token_e type);

int sa_parse_map(sa_map_s *map, buf_s src) {
	map_token_s *tok;		
	char *ptr = src.data;

	while((tok = map_next_tok(&ptr))->type != MAP_TOK_EOF) {
		log_info("token: %s type: %d", tok->lex, tok->type);
	}

}

map_token_s *map_next_tok(char **ptr) {
	bool got_alpha;
	char *fptr = *ptr, *bptr, bck;
	map_token_s *t;

	while(isspace(*fptr))
		fptr++;
	switch(*fptr) {
		case '=':
			t =  map_token_init("=", 1, MAP_TOK_EQ);
			fptr++;
			break;
		case '&':
			t = map_token_init("&", 1, MAP_TOK_AMP);
			fptr++;
			break;
		case '\0':
			t = map_token_init("EOF", 3, MAP_TOK_EOF);
			break;
		default:
			if(isdigit(*fptr)) {
				got_alpha = false;
				bptr = fptr++;
				while(true) {
					if(isalpha(*fptr) || *fptr == '_') {
						got_alpha = true;
					}
					else if(!isdigit(*fptr)) {
						break;
					}
					fptr++;
				}
				bck = *fptr;
				*fptr = '\0';
				if(got_alpha) 
					t = map_token_init(bptr, fptr - bptr, MAP_TOK_IDENT);
				else
					t = map_token_init(bptr, fptr - bptr, MAP_TOK_INT);
				*fptr = bck;
			}
			else if(isalnum(*fptr) || *fptr == '_') {
				bptr = fptr++;
				while(isalnum(*fptr) || *fptr == '_') 
					fptr++;
				bck = *fptr;
				*fptr = '\0';
				t = map_token_init(bptr, fptr - bptr, MAP_TOK_IDENT);
				*fptr = bck;
			}
			break;
	}
	*ptr = fptr;
	return t;
}

map_token_s *map_token_init(char *lexeme, int len, map_token_e type) {
	map_token_s *t;

	t = sa_alloc(sizeof *t);
	if(len >= MAX_LEX_LEN) {
		log_error("Error parsing sa map file - lexeme too long %s.", lexeme);
		return NULL;
	}
	strcpy(t->lex, lexeme);
	t->type = type;
	return t;
}

