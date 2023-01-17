# Makefile for Sniffer project

all: partA partAforC Codes_Packet_Sniffing_Spoofing

#Codes_Packet_Sniffing_Spoofing: Codes_Packet_Sniffing_Spoofing.c
#	gcc -o Codes Codes_Packet_Sniffing_Spoofing.c

Codes_Packet_Sniffing_Spoofing: Codes_Packet_Sniffing_Spoofing.c
	gcc -g -o Codes Codes_Packet_Sniffing_Spoofing.c -lpcap

partA: partA.c
	gcc -o partA partA.c

partAforC: partAforC.c
	gcc -o partAforC partAforC.c


clean:
	rm -f *.o partA partAforC Codes_Packet_Sniffing_Spoofing