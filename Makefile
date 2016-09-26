VERSION=1.0.3

CFLAGS=-std=c99 -Wall -pedantic -O3

PREFIX_32=serial-tools-$(VERSION)-i386
PREFIX_64=serial-tools-$(VERSION)-x86-64

CFLAGS_32=$(CFLAGS) -m32
CFLAGS_64=$(CFLAGS)

all: rs232cat tunslip trillium

release: $(PREFIX_64).tar.gz $(PREFIX_32).tar.gz

rs232cat: rs232cat.c rs232.c rs232.h options.c options.h io.c io.h Makefile
	$(CC) -o rs232cat rs232cat.c rs232.c options.c io.c $(CFLAGS) -DVERSION="\"$(VERSION)\""

tunslip: tunslip.c tun.c tun.h rs232.c rs232.h options.c options.h io.c io.h Makefile
	$(CC) -o tunslip tun.c tunslip.c rs232.c options.c io.c $(CFLAGS) -DVERSION="\"$(VERSION)\""

trillium: tunslip
	cp tunslip trillium

$(PREFIX_64).tar.gz: $(PREFIX_64)/rs232cat $(PREFIX_64)/tunslip $(PREFIX_64)/trillium $(PREFIX_64)/LICENSE $(PREFIX_64)/README.md $(PREFIX_64)/Makefile
	tar czf $(PREFIX_64).tar.gz $(PREFIX_64)/rs232cat $(PREFIX_64)/tunslip $(PREFIX_64)/trillium $(PREFIX_64)/LICENSE $(PREFIX_64)/README.md $(PREFIX_64)/Makefile

$(PREFIX_32).tar.gz: $(PREFIX_32)/rs232cat $(PREFIX_32)/tunslip $(PREFIX_32)/trillium $(PREFIX_32)/LICENSE $(PREFIX_32)/README.md $(PREFIX_32)/Makefile
	tar czf $(PREFIX_32).tar.gz $(PREFIX_32)/rs232cat $(PREFIX_32)/tunslip $(PREFIX_32)/trillium $(PREFIX_32)/LICENSE $(PREFIX_32)/README.md $(PREFIX_32)/Makefile

$(PREFIX_64)/rs232cat: rs232cat.c rs232.c rs232.h options.c options.h io.c io.h Makefile
	@mkdir -p $(PREFIX_64)/
	$(CC) -o $(PREFIX_64)/rs232cat rs232cat.c rs232.c options.c io.c $(CFLAGS_64) -DVERSION="\"$(VERSION)\""

$(PREFIX_64)/tunslip: tunslip.c tun.c tun.h rs232.c rs232.h options.c options.h io.c io.h Makefile
	@mkdir -p $(PREFIX_64)/
	$(CC) -o $(PREFIX_64)/tunslip tun.c tunslip.c rs232.c options.c io.c $(CFLAGS_64) -DVERSION="\"$(VERSION)\""

$(PREFIX_64)/trillium: $(PREFIX_64)/tunslip
	@mkdir -p $(PREFIX_64)/
	cp $(PREFIX_64)/tunslip $(PREFIX_64)/trillium

$(PREFIX_64)/LICENSE: LICENSE
	@mkdir -p $(PREFIX_64)/
	cp LICENSE $(PREFIX_64)/LICENSE

$(PREFIX_64)/README.md: README.md
	@mkdir -p $(PREFIX_64)/
	cp README.md $(PREFIX_64)/README.md

$(PREFIX_64)/Makefile: install.mk
	@mkdir -p $(PREFIX_64)/
	cp install.mk $(PREFIX_64)/Makefile

$(PREFIX_32)/rs232cat: rs232cat.c rs232.c rs232.h options.c options.h io.c io.h Makefile
	@mkdir -p $(PREFIX_32)/
	$(CC) -o $(PREFIX_32)/rs232cat rs232cat.c rs232.c options.c io.c $(CFLAGS_32) -DVERSION="\"$(VERSION)\""

$(PREFIX_32)/tunslip: tunslip.c tun.c tun.h rs232.c rs232.h options.c options.h io.c io.h Makefile
	@mkdir -p $(PREFIX_32)/
	$(CC) -o $(PREFIX_32)/tunslip tun.c tunslip.c rs232.c options.c io.c $(CFLAGS_32) -DVERSION="\"$(VERSION)\""

$(PREFIX_32)/trillium: $(PREFIX_32)/tunslip
	@mkdir -p $(PREFIX_32)/
	cp $(PREFIX_32)/tunslip $(PREFIX_32)/trillium

$(PREFIX_32)/LICENSE: LICENSE
	@mkdir -p $(PREFIX_32)/
	cp LICENSE $(PREFIX_32)/LICENSE

$(PREFIX_32)/Makefile: install.mk
	@mkdir -p $(PREFIX_32)/
	cp install.mk $(PREFIX_32)/Makefile

$(PREFIX_32)/README.md: README.md
	@mkdir -p $(PREFIX_32)/
	cp README.md $(PREFIX_32)/README.md

include install.mk
