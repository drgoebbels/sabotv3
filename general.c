#include "general.h"
#include <stdio.h>
#include <string.h>

struct map_record_s {
	const char *key;
	void *value;
	map_record_s *next;
};

static unsigned pjw_hash(const char *key);

map_s *map_alloc(void) {
	return sa_allocz(sizeof(map_s));
}

int map_insert(map_s *map, const char *key, void *value) {
	map_record_s **prec = &map->table[pjw_hash(key)],
				 *rec = *prec,
				 *nrec;
	nrec = sa_alloc(sizeof *nrec);
	nrec->key = key;
	nrec->value = value;
	nrec->next = NULL;
	if(rec) {
		while(rec->next) {
			if(!strcmp(rec->key, key)) {
				free(nrec);
				return -1;
			}
			rec = rec->next;
		}
		if(!strcmp(rec->key, key)) {
			free(nrec);
			return -1;
		}
		rec->next = nrec;
	}
	else {
		*prec = nrec;
	}
	return 0;
}

void *map_get(map_s *map, const char *key) {
	map_record_s *rec = map->table[pjw_hash(key)];

	while(rec) {
		if(!strcmp(rec->key, key)) {
			return rec->value;
		}
		rec = rec->next;
	}
	return NULL;
}

int map_delete(map_s *map, const char *key) {
	map_record_s **prec = &map->table[pjw_hash(key)],
				 *rec = *prec, *bptr;

	for(bptr = rec; rec; rec = rec->next) {
		if(!strcmp(rec->key, key)) {
			if(rec == bptr) {
				*prec = rec->next;
			}
			else {
				bptr->next = rec->next;
			}
			free(rec);
			return 0;
		}
		bptr = rec;
	}
	return -1;
}

void map_dealloc(map_s *map) {
	int i;
	map_record_s *rec, *bptr;

	for(i = 0; i < HASH_TABLE_SIZE; i++) {
		rec = map->table[i];
		while(rec) {
			bptr = rec->next;
			free(rec);
			rec = bptr;
		}
	}
	free(map);
}

void *sa_alloc(size_t size) {
	void *p = malloc(size);
	if(!p) {
		perror("Memory Allocation Error - malloc()");
		exit(EXIT_FAILURE);
	}
	return p;
}

void *sa_allocz(size_t size) {
	void *p = calloc(1, size);
	if(!p) {
		perror("Memory Allocation Error - calloc()");
		exit(EXIT_FAILURE);
	}
	return p;
}

void *sa_ralloc(void *ptr, size_t size) {
	void *p = realloc(ptr, size);
	if(!p) {
		perror("Memory Allocation Error - realloc()");
		exit(EXIT_FAILURE);
	}
	return p;
}

unsigned pjw_hash(const char *key) {
	unsigned h = 0, g;
	while(*key) {
		h = (h << 4) + *key++;
		if((g = h & (unsigned)0xF0000000) != 0) {
			h = (h ^ (g >> 4)) ^g;
		}
	}
	return (unsigned)(h % HASH_TABLE_SIZE);
}

