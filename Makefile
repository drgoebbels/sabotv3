out: 
	cc -ggdb -lsqlite3 -lpthread pool.c socks5.c server.c db.c log.c net-general.c general.c sa-chatroom.c main.c -o sa

db-schema.c:
	schema := $(shell cat db/schema.sql | sed 's/"/\\"/g | perl -ne '/^(.*)$/ && print "\"$1\"\n"') 
	@echo "done"
			
