# Makefile for TCP project

all: sender reciever tcp_client

reciever: reciever.c
	gcc -o reciever reciever.c

sender: sender.c
	gcc -o sender sender.c


tcp_client: tcp_client.c
	gcc -o tcp_client tcp_client.c


clean:
	rm -f *.o reciever sender