#include "general.h"
#include <stdio.h>
#include <string.h>

struct map_record_s {
	const char *key;
	void *value;
	map_record_s *next;
};

static unsigned pjw_hash(const char *key);

void map_init(map_s *map) {
    int i;
    map_record_s **ptr;

    map->size = 0;
    for(i = 0, ptr = map->table; i < HASH_TABLE_SIZE; i++) {
        *ptr++ = 0;
    }
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
	map->size++;
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

void *map_delete(map_s *map, const char *key) {
	map_record_s **prec = &map->table[pjw_hash(key)],
				 *rec = *prec, *bptr;

	for(bptr = rec; rec; rec = rec->next) {
		if(!strcmp(rec->key, key)) {
			void *value = rec->value;
			if(rec == bptr) {
				*prec = rec->next;
			}
			else {
				bptr->next = rec->next;
			}
			free(rec);
			map->size--;
			return value;
		}
		bptr = rec;
	}
	return NULL;
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
}

void buf_init(buf_s *buf) {
	buf->data = sa_alloc(INITIAL_BUF_SIZE);
	buf->bsize = INITIAL_BUF_SIZE;
	buf->size = 0;
}

void buf_add_char(buf_s *buf, char c) {
	char *raw;
	
	if(buf->size == buf->bsize) {
		buf->bsize *= 2;
		buf->data = sa_ralloc(buf->data, buf->bsize);
	}
	raw = buf->data;
	raw[buf->size++] = c;
}

void buf_add_int(buf_s *buf, int i) {
	size_t real_size = sizeof(i) * buf->size;
	size_t new_size = real_size + sizeof(i);
	int *raw;
	
	if(new_size >= buf->bsize) {
		buf->bsize *= 2;
		buf->data = sa_ralloc(raw, buf->bsize); 
	}
	raw = buf->data;
	raw[buf->size++] = i;
}

void buf_dealloc(buf_s *buf) {
	free(buf->data);
}

buf_s read_file(const char *file_name) {
	int c;
	FILE *f;
	buf_s buf;

	f = fopen(file_name, "r");
	if(!f) {
		perror("failed to open file");
		buf.data = NULL;
	}
	buf_init(&buf);
	while((c = fgetc(f)) != EOF)
		buf_add_char(&buf, c);
	buf_add_char(&buf, '\0');
	fclose(f);
	return buf;
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

