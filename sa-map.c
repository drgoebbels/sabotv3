#include "sa-map.h"
#include "log.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define INITIAL_TILE_LIST_SIZE 512
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
static int sa_parse_heading(sa_map_s *map, char **ptr);
static int sa_parse_tiles(sa_map_s *map, char **ptr);
static int sa_parse_tile_list(sa_map_s *map, char **ptr);
static int sa_parse_intlist(char **ptr, const char *id, buf_s *buf);
static int sa_parse_int(char **ptr, const char *id, int *i);

int sa_parse_map(sa_map_s *map, buf_s src) {
    int result;
	map_token_s tok;		
	char *ptr;

	ptr = src.data;	
	result = sa_parse_heading(map, &ptr);
	if(result) 
		return -1;
	result = sa_parse_tiles(map, &ptr);
	if(result)
		return -1;
    result = sa_parse_intlist(&ptr, "sp", &map->sp);
    if(result)
        return -1;
    result = sa_parse_intlist(&ptr, "ws", &map->ws);
    if(result)
        return -1;
    result = sa_parse_int(&ptr, "rt", &map->rt);
    if(result)
        return -1;
    result = sa_parse_int(&ptr, "ts", &map->rt);
    if(result)
        return -1;
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

int sa_parse_tiles(sa_map_s *map, char **ptr) {
	int result;
	map_token_s tok;

	result = map_next_tok(&tok, ptr);
	if(result)
		return -1;
	if(tok.type == MAP_TOK_IDENT && !strcmp(tok.lex, "tiles")) {
		result = map_next_tok(&tok, ptr);
		if(result)
			return -1;
		if(tok.type != MAP_TOK_EQ) {
			log_error("Syntax Error parsing sa map in %s(): expected '=' but got %s.", __func__, tok.lex);
			return -1;
		}
		return sa_parse_tile_list(map, ptr);
	}
	else {
		log_error("Syntax Error parsing sa map in %s(): unexpected token: %s", __func__, tok.lex);
		return -1;
	}
}

int sa_parse_tile_list(sa_map_s *map, char **ptr) {
	int tile_list_size = 0,
        product = map->infx * map->infy,
		result;
	map_token_s tok;
	sa_tile_s tile, *tile_buffer;
	
	tile_buffer = sa_alloc(product * sizeof(*tile_buffer));
	while(true) {
		result = map_next_tok(&tok, ptr);
		if(result == -1) {
			free(tile_buffer);
			return -1;
		}
		if(tok.type == MAP_TOK_AMP) {
			if(tile_list_size != product) {
				log_error("Unexpected tile list size in parsed map in %s(). Expected %d, but got %d tiles.", __func__, product, tile_list_size);
				free(tile_buffer);
				return -1;
			}
			tile_buffer = sa_ralloc(tile_buffer, tile_list_size * sizeof(*tile_buffer));
			map->tiles = tile_buffer;
			return 0;
		}
		else if(tok.type == MAP_TOK_IDENT) {
			if(tok.len >= SA_TILE_LEN) {
				log_error("Error parsing map file in %s(): tile too large: %s", __func__, tok.lex);
				free(tile_buffer);
				return -1;
			}
            if(tile_list_size >= product) {
                log_error("More tiles than expected found in %s()", __func__);
                return -1;
            }
			strcpy(tile_buffer[tile_list_size].id, tok.lex);
			tile_list_size++;
		}
		else {
			log_error("Syntax Error parsing sa map in %s(): expected identifier but got %s", __func__, tok.lex);
			free(tile_buffer);
			return -1;
		}
	}	

}

int sa_parse_intlist(char **ptr, const char *id, buf_s *buf) {
    int result, i;
    map_token_s tok;

    result = map_next_tok(&tok, ptr);
    if(result == -1)
        return -1;
    if(tok.type == MAP_TOK_IDENT && !strcmp(tok.lex, id)) {
        result = map_next_tok(&tok, ptr);
        if(result)
            return -1;
        if(tok.type != MAP_TOK_EQ) {
            log_error("Syntax Error parsing sa map in %s(): expected '=' but got: %s", __func__, tok.lex);
            return -1;
        }
        buf_init(buf);
        while(true) {
            result = map_next_tok(&tok, ptr);
            if(result) {
                buf_dealloc(buf);
                return -1;
            }
            if(tok.type == MAP_TOK_INT) {
               i = atoi(tok.lex); 
               buf_add_int(buf, i);
            } 
            else if(tok.type == MAP_TOK_AMP) {
                return 0;
            }
            else {
                buf_dealloc(buf);
                log_error("Syntax Error parsing sa map in %s(): expected integer but got %s.", __func__, tok.lex);
                return -1;
            }
        }
    }
    else {
        log_error("Syntax Error parsing sa map in %s(): expected %s identifier but got %s", __func__, id, tok.lex);
        return -1;
    }
}


int sa_parse_int(char **ptr, const char *id, int *i) {
    int result;
    map_token_s tok;

    result = map_next_tok(&tok, ptr);
    if(result)
        return -1;
    if(tok.type == MAP_TOK_IDENT && !strcmp(tok.lex, id)) {
        result = map_next_tok(&tok, ptr);
        if(result)
            return -1;
        if(tok.type != MAP_TOK_EQ) {
            log_error("Syntax Error parsing sa map in %s(): expected '=' but got: %s", __func__, tok.lex);
            return -1;
        }
        result = map_next_tok(&tok, ptr);
        if(result)
            return -1;
        if(tok.type != MAP_TOK_INT) {
            if(tok.type == MAP_TOK_IDENT && !strcmp(tok.lex, "undefined")) {
                *i = -1;
            }
            else {
                log_error("Syntax Error parsing sa map in %s(): expected integer but got: %s", __func__, tok.lex);
                return -1;
            }
        }
        else {
            *i = atoi(tok.lex);
        }
        result = map_next_tok(&tok, ptr);
        if(result)
            return -1;
        if(tok.type != MAP_TOK_AMP && tok.type != MAP_TOK_EOF) {
            log_error("Syntax error parsing sa map in %s(): expected '&' or EOF, but got %s", __func__, tok.lex);
            return -1;
        }
        return 0;
    }
    else {
        log_error("Syntax Error parsing sa map in %s(): expected identifier %s but got %s.", __func__, id, tok.lex);
        return -1;
    }
}

void sa_map_dealloc(sa_map_s *map) {
    buf_dealloc(&map->sp);
    buf_dealloc(&map->ws);
    free(map->tiles);
}

