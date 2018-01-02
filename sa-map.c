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
	int len;
	map_token_e type;
	char lex[MAX_LEX_LEN];
};

static void map_lex(char *src);
static int map_next_tok(map_token_s *tok, char **ptr);
static int map_token_init(map_token_s *tok, char *lexeme, int len, map_token_e type);
static int sa_build_map(sa_map_s *map, char **ptr);
static int sa_parse_heading(sa_map_s *map, char **ptr);

int sa_parse_map(sa_map_s *map, buf_s src) {
	map_token_s *tok;		
	char *data;

	data = src.data;	
	sa_build_map(map, &data);
	log_info("infx: %d infy: %d name: '%s'", map->infx, map->infy, map->name);

	return 0;
}

int map_next_tok(map_token_s *tok, char **ptr) {
	bool got_alpha;
	int result;
	char *fptr = *ptr, *bptr, bck;

	while(isspace(*fptr))
		fptr++;
	switch(*fptr) {
		case '=':
			result =  map_token_init(tok, "=", 1, MAP_TOK_EQ);
			fptr++;
			break;
		case '&':
			result = map_token_init(tok, "&", 1, MAP_TOK_AMP);
			fptr++;
			break;
		case '\0':
			result = map_token_init(tok, "EOF", 3, MAP_TOK_EOF);
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
					result = map_token_init(tok, bptr, fptr - bptr, MAP_TOK_IDENT);
				else
					result = map_token_init(tok, bptr, fptr - bptr, MAP_TOK_INT);
				*fptr = bck;
			}
			else if(isalnum(*fptr) || *fptr == '_') {
				bptr = fptr++;
				while(isalnum(*fptr) || *fptr == '_') 
					fptr++;
				bck = *fptr;
				*fptr = '\0';
				result = map_token_init(tok, bptr, fptr - bptr, MAP_TOK_IDENT);
				*fptr = bck;
			}
			else {
				log_error("Lexical Error Parsing sa map file in %s() - Unrecognized char: %c", __func__, *fptr);
				fptr++;
				result = -1;
			}
			break;
	}
	*ptr = fptr;
	return result;
}

int map_token_init(map_token_s *t, char *lexeme, int len, map_token_e type) {
	if(len >= MAX_LEX_LEN) {
		log_error("Lexical Error parsing sa map file in %s() - lexeme too long %s.", __func__, lexeme);
		return -1;
	}
	strcpy(t->lex, lexeme);
	t->type = type;
	t->len = len;
	return 0;
}

int sa_build_map(sa_map_s *map, char **ptr) {
	int result;	

	result = sa_parse_heading(map, ptr);
	if(result) 
		return -1;
}

int sa_parse_heading(sa_map_s *map, char **ptr) {
	int result, count;
	map_token_s tok;

	result = map_next_tok(&tok, ptr);
	if(result)
		return -1;
	if(tok.type == MAP_TOK_IDENT && !strcmp(tok.lex, "inf")) {
		result = map_next_tok(&tok, ptr);
		if(result) 
			return -1;
		if(tok.type != MAP_TOK_EQ) {
			log_error("Syntax Error parsing sa map in %s(): expected '=', but got: %s", __func__, tok.lex);
			return -1;
		}
		result = map_next_tok(&tok, ptr);
		if(result)
			return -1;
		if(tok.type != MAP_TOK_INT) {
			log_error("Syntax Error parsing sa map in %s(): expected integer, but got: %s", __func__, tok.lex);
			return -1;
		}
		map->infx = atoi(tok.lex);
		result = map_next_tok(&tok, ptr);
		if(result)
			return -1;
		if(tok.type != MAP_TOK_INT) {
			log_error("Syntax Error parsing sa map in %s(): expected integer, but got: %s", __func__, tok.lex);
			return -1;
		}
		map->infy = atoi(tok.lex);
		count = 0;
		while(true) {
			result = map_next_tok(&tok, ptr);
			if(result)
				return -1;
			if(tok.type == MAP_TOK_AMP) {
				return 0;
			}
			else {
				if(count > 0) {
					if(count >= MAX_ARENA_NAME_SIZE - 1) {
						log_error("Error parsing sa map in %s(): game name too long: %s", __func__, tok.lex);
						return -1;
					}
					map->name[count++] = ' ';
				}
				if(count + tok.len + 1 >= MAX_ARENA_NAME_SIZE) {
						log_error("Error parsing sa map in %s(): game name too long: %s", __func__, tok.lex);
						return -1;
				}
				strcpy(&map->name[count], tok.lex);
				count += tok.len;
			}
		}
		log_info("got map name: %s", map->name);
	}
	else {
		log_error("Syntax Error parsing sa map in %s(): unexpected token: %s", __func__, tok.lex);
		return -1;
	}
}

