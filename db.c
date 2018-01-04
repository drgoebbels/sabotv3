#include "db.h"
#include "log.h"
#include <sqlite3.h>

static sqlite3 *db_handle;


static int db_prepare_queries(void);

int db_init(const char *name) {
	int result;

	log_debug("Opening database file: %s", name);
	result = sqlite3_open(name, &db_handle);
	if(result != SQLITE_OK) {
		log_error("Failed to start up database: %s\n", name);
		return -1;
	}
	return 0;
}

int db_prepare_queries(void) {
}

int db_check_dns_cache(const char *host_name, char *address_buffer) {
	return 0;
}

int db_insert_dns_entry(const char *host_name, const char *address) {
	return 0;
}

