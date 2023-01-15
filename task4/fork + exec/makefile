all: ping watchdog new_ping myping
ping: ping.c
	gcc ping.c -o parta
watchdog: watchdog.c
	gcc watchdog.c -o watchdog
new_ping: new_ping.c
	gcc new_ping.c -o partb
myping: myping.c
	gcc -o myping myping.c

clean:
	rm -f *.o parta watchdog partb myping
