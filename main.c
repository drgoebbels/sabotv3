#include "log.h"
#include <stdio.h>

int main(void) {
	log_init_name("test.log");
	log_info("hello turd");
	log_end();
	return 0;
}


