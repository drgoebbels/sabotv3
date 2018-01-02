#ifndef __general_h__
#define __general_h__

#include <stdlib.h>

#define ERR_BUF_SIZE 512
#define HASH_TABLE_SIZE 97
#define INITIAL_BUF_SIZE 128

typedef struct map_s map_s;
typedef struct map_record_s map_record_s;
typedef struct buf_s buf_s;

struct map_s {
	size_t size;
	map_record_s *table[HASH_TABLE_SIZE]; 
};

struct buf_s {
	size_t bsize;
	size_t size; 
	void *data;
};

extern void map_init(map_s *map);
extern int map_insert(map_s *map, const char *key, void *value);
extern void *map_get(map_s *map, const char *key);
extern void *map_delete(map_s *map, const char *key);
extern void map_dealloc(map_s *map);

extern void buf_init(buf_s *buf);
extern void buf_add_char(buf_s *buf, char c);
extern void buf_add_int(buf_s *buf, int i);
extern void buf_dealloc(buf_s *buf);

extern buf_s read_file(const char *file_name);

extern void *sa_alloc(size_t size);
extern void *sa_allocz(size_t size);
extern void *sa_ralloc(void *ptr, size_t size);

#endif

