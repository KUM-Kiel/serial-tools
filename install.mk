PREFIX = /usr/local

install: rs232cat tunslip trillium
	install -d "$(PREFIX)/bin"
	install -m 0755 rs232cat "$(PREFIX)/bin"
	install -m 4755 tunslip "$(PREFIX)/bin"
	install -m 4755 trillium "$(PREFIX)/bin"
