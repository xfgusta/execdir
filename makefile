CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c11 -O2

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/share/man

INSTALL=install -p -m 0755
INSTALL_MAN=install -p -m 0644

execdir: execdir.c
	$(CC) $(CFLAGS) execdir.c $(LDFLAGS) -o execdir

install: execdir
	mkdir -p $(DESTDIR)$(BINDIR)
	$(INSTALL) execdir $(DESTDIR)$(BINDIR)
	ln -f -s execdir $(DESTDIR)$(BINDIR)/x
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	$(INSTALL_MAN) execdir.1 $(DESTDIR)$(MANDIR)/man1
	ln -f -s execdir.1 $(DESTDIR)$(MANDIR)/man1/x.1

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/execdir
	rm -f $(DESTDIR)$(BINDIR)/x
	rm -f $(DESTDIR)$(MANDIR)/man1/execdir.1
	rm -f $(DESTDIR)$(MANDIR)/man1/x.1

clean:
	rm -f execdir

.PHONY: install uninstall clean
