CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g

dhcplr: dhcplr.o

clean:
	rm -f dhcplr dhcplr.o
