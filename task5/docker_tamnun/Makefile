# variables
CC = gcc -Wall

# units
Sniffer.o: Sniffer.c
	$(CC) -o Sniffer.o -c Sniffer.c

Spoofer.o: Spoofer.c
	$(CC) -o Spoofer.o -c Spoofer.c

SnifferSpoofer.o: SnifferSpoofer.c
	$(CC) -o SnifferSpoofer.o -c SnifferSpoofer.c

Gateway.o: Gateway.c
	$(CC) -o Gateway.o -c Gateway.c

# applications
Sniffer: Sniffer.o
	$(CC) -o Sniffer Sniffer.o -lpcap

Spoofer: Spoofer.o
	$(CC) -o Spoofer Spoofer.o -lpcap

SnifferSpoofer: SnifferSpoofer.o
	$(CC) -o SnifferSpoofer SnifferSpoofer.o -lpcap

Gateway: Gateway.o
	$(CC) -o Gateway Gateway.o -lpcap

# CI/CD
clean:
	rm -rf *.o Sniffer Spoofer SnifferSpoofer Gateway

docker-build:
	docker-compose build

docker-up:
	docker-compose up -d

docker-rebuild:
	docker-compose build
	docker-compose up -d

# build
build: Sniffer Spoofer SnifferSpoofer Gateway

all: clean build

# testing
test-example:
	docker-compose exec attacker ./Sniffer

test-example-2:
	docker-compose exec attacker ./Spoofer arg-a arg-b
