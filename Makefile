VERSION = 1.1.0
DATE = 2017-09-28
PLATFORM ?= linux

CFLAGS += -std=c99 -Wall -pedantic -Os

RELEASE = serial-tools-$(DATE)-$(VERSION)-$(PLATFORM)

STRIP ?= strip

all: rs232cat tunslip trillium

release: $(RELEASE).tar.gz

rs232cat: rs232cat.c rs232.c rs232.h options.c options.h io.c io.h Makefile
	$(CC) -o rs232cat rs232cat.c rs232.c options.c io.c $(CFLAGS) -DVERSION="\"$(VERSION) ($(DATE))\""

tunslip: tunslip.c tun.c tun.h rs232.c rs232.h options.c options.h io.c io.h Makefile
	$(CC) -o tunslip tun.c tunslip.c rs232.c options.c io.c $(CFLAGS) -DVERSION="\"$(VERSION) ($(DATE))\""

trillium: tunslip
	cp tunslip trillium

$(RELEASE).tar.gz: $(RELEASE)/rs232cat $(RELEASE)/tunslip $(RELEASE)/trillium $(RELEASE)/LICENSE $(RELEASE)/README.md $(RELEASE)/Makefile
	tar czf $(RELEASE).tar.gz $(RELEASE)/rs232cat $(RELEASE)/tunslip $(RELEASE)/trillium $(RELEASE)/LICENSE $(RELEASE)/README.md $(RELEASE)/Makefile

$(RELEASE)/rs232cat: rs232cat Makefile
	@mkdir -p $(RELEASE)/
	@cp rs232cat $(RELEASE)/rs232cat
	@$(STRIP) $(RELEASE)/rs232cat

$(RELEASE)/tunslip: tunslip Makefile
	@mkdir -p $(RELEASE)/
	@cp tunslip $(RELEASE)/tunslip
	@$(STRIP) $(RELEASE)/tunslip

$(RELEASE)/trillium: $(RELEASE)/tunslip
	@mkdir -p $(RELEASE)/
	@cp $(RELEASE)/tunslip $(RELEASE)/trillium

$(RELEASE)/LICENSE: LICENSE
	@mkdir -p $(RELEASE)/
	@cp LICENSE $(RELEASE)/LICENSE

$(RELEASE)/README.md: README.md
	@mkdir -p $(RELEASE)/
	@cp README.md $(RELEASE)/README.md

$(RELEASE)/Makefile: install.mk
	@mkdir -p $(RELEASE)/
	@cp install.mk $(RELEASE)/Makefile

include install.mk

clean:
	rm -rf serial-tools-* rs232cat tunslip trillium
