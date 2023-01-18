# Makefile for Sniffer project
all: Sniffer Spoofer Gateway attacker

Sniffer: Sniffer.c
	gcc -o Sniffer Sniffer.c

Spoofer: Spoofer.c
	gcc -o Spoofer Spoofer.c

Gateway: Gateway.c
	gcc -o Gateway Gateway.c -lpcap

attacker: attacker.c
	gcc -o attacker attacker.c -lpcap
	
clean:
	rm -f *.o Sniffer Gateway attacker Spoofer