#ifndef __general_h__
#define __general_h__

#include <stdlib.h>

#define ERR_BUF_SIZE 512
#define HASH_TABLE_SIZE 97

typedef struct map_s map_s;
typedef struct map_record_s map_record_s;

struct map_s {
	int size;
	map_record_s *table[HASH_TABLE_SIZE]; 
};

extern void map_init(map_s *map);
extern int map_insert(map_s *map, const char *key, void *value);
extern void *map_get(map_s *map, const char *key);
extern void *map_delete(map_s *map, const char *key);
extern void map_dealloc(map_s *map);

extern void *sa_alloc(size_t size);
extern void *sa_allocz(size_t size);
extern void *sa_ralloc(void *ptr, size_t size);

#endif

