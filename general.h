#ifndef __general_h__
#define __general_h__

#include <stdlib.h>

#define ERR_BUF_SIZE 512

extern void *sa_alloc(size_t size);
extern void *sa_allocz(size_t size);
extern void *sa_ralloc(void *ptr, size_t size);

#endif

