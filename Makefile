out:
	cc -ggdb -lpthread pool.c socks5.c server.c db.c log.c net-general.c general.c sa-chatroom.c main.c -o sa
