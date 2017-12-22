out:
	gcc -ggdb -lpthread pool.c socks5.c server.c db.c log.c general.c chatroom.c main.c -o sa
