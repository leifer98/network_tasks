# Makefile for TCP project

all: tcp_server tcp_client udp_server udp_client

tcp_server: tcp_server.c
	gcc -o tcp_server tcp_server.c

tcp_client: tcp_client.c
	gcc -o tcp_client tcp_client.c

udp_server: udp_server.c
	gcc -o udp_server udp_server.c

udp_client: udp_client.c
	gcc -o udp_client udp_client.c

clean:
	rm -f *.o tcp_server tcp_client udp_server udp_client