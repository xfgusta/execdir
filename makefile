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
	ln -f -s $(DESTDIR)$(BINDIR)/execdir $(DESTDIR)$(BINDIR)/xdir
	mkdir -p $(DESTDIR)$(MANDIR)/man1
	$(INSTALL_MAN) execdir.1 $(DESTDIR)$(MANDIR)/man1
	ln -f -s $(DESTDIR)$(MANDIR)/man1/execdir.1 $(DESTDIR)$(MANDIR)/man1/xdir.1

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/execdir
	rm -f $(DESTDIR)$(BINDIR)/xdir
	rm -f $(DESTDIR)$(MANDIR)/man1/execdir.1
	rm -f $(DESTDIR)$(MANDIR)/man1/xdir.1

clean:
	rm -f execdir

