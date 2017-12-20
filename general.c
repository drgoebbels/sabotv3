#include "general.h"
#include <stdio.h>

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


