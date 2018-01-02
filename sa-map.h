#ifndef __sa_map_h__
#define __sa_map_h__

#include "general.h"

typedef struct sa_map_s sa_map_s;

struct sa_map_s {
	int infx, infy;
	char name[64];
	buf_s tiles;
	buf_s sp;
	buf_s ws;
	int rt;
	int ts;
};

extern int sa_parse_map(sa_map_s *map, buf_s src);

#endif

