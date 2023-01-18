# Makefile for Sniffer project

all: partA partAforC pcap attacker

#Codes_Packet_Sniffing_Spoofing: Codes_Packet_Sniffing_Spoofing.c
#	gcc -o Codes Codes_Packet_Sniffing_Spoofing.c

pcap: pcap.c
	gcc -g -o pcap pcap.c -lpcap

attacker: attacker.c
	g++ -o attacker attacker.c

partA: partA.c
	gcc -o partA partA.c

partAforC: partAforC.c
	gcc -o partAforC partAforC.c


clean:
	rm -f *.o partA partAforC pcap attacker