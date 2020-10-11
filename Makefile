all:	openvpn-plugin-python.so

WARNINGS=-Wall
CC=gcc
GCC=gcc
#WARNINGS=-Wunreachable-code 	# Gcc 4.1 .. 4.4 are too buggy to make this useful

LIBDIR ?= /usr/local/lib/openvpn/plugins

openvpn-plugin-python.so: openvpn-plugin-python.c embedpython.c Makefile
	@echo rm -f "$@"
	@[ ! -e build -o build/lib.*/$@ -nt setup.py -a build/lib.*/$@ -nt Makefile ] || rm -r build
	CFLAGS="$(WARNINGS) -I/usr/local/lib/" ./setup.py build
	@#CFLAGS="-O0 $(WARNINGS)" ./setup.py build
	ln -sf build/lib.*/$@ .

.PHONY: install install-lib
install: install-lib
install-lib:
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp build/lib.*/openvpn-plugin-python.so $(DESTDIR)$(LIBDIR)
	CFLAGS="$(WARNINGS) -I/usr/local/lib/" ./setup.py install

.PHONY: clean
clean:
	rm -rf openvpn-plugin-python.so build dist core
	CFLAGS="$(WARNINGS) -I/usr/local/lib/" ./setup.py clean
