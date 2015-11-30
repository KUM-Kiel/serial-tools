all: rs232cat tunslip trillium

rs232cat: rs232cat.c rs232.c rs232.h options.c options.h io.c io.h Makefile
	$(CC) -o rs232cat rs232cat.c rs232.c options.c io.c -std=c99 -Wall -pedantic

tunslip: tunslip.c tun.c tun.h rs232.c rs232.h options.c options.h io.c io.h Makefile
	$(CC) -o tunslip tun.c tunslip.c rs232.c options.c io.c -std=c99 -Wall -pedantic

trillium: tunslip
	cp tunslip trillium
