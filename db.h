#ifndef __db_h__
#define __db_h__


extern int db_init(const char *name);

extern int db_check_dns_cache(const char *host_name, char *address_buffer);
extern int db_insert_dns_entry(const char *host_name, const char *address);

#endif


