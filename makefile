CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c11 -O2

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

INSTALL=install -p -m 0755

execdir: execdir.c
	$(CC) $(CFLAGS) execdir.c $(LDFLAGS) -o execdir

install: execdir
	mkdir -p $(DESTDIR)$(BINDIR)
	$(INSTALL) execdir $(DESTDIR)$(BINDIR)
	ln -s $(DESTDIR)$(BINDIR)/execdir $(DESTDIR)$(BINDIR)/xdir

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/execdir
	rm -f $(DESTDIR)$(BINDIR)/xdir

clean:
	rm -f execdir

