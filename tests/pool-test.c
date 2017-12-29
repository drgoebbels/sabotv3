#include "../pool.h"
#include "../general.h"
#include <stdlib.h>

#define DEFAULT_TEST_SIZE 1000
#define DEFAULT_POOL_SIZE 50

int main(int argc, char *argv[]) {
	int test_size, pool_size;

	if(argc == 2) {
		test_size = atoi(argv[1]);
		pool_size = DEFAULT_POOL_SIZE;
	}
	else if(argc == 3) {
		test_size = atoi(argv[1]);
		pool_size = atoi(argv[2]);
	}
	else {
		test_size = DEFAULT_TEST_SIZE;
		pool_size = DEFAULT_POOL_SIZE;
	}
	pool_task_s *tasks = sa_alloc(sizeof(*tasks) * test_size);
	pool_s *pool = pool_init(pool_size);

	pool_shutdown(pool);
	free(tasks);
	return 0;
}

