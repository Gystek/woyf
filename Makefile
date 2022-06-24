# woyf - watch over your files

include config.mk

SRC=main.c
OBJ=$(SRC:.c=.o)

all: woyf
woyf: $(OBJ)
	$(CC) $(LDFLAGS) -o woyf $(OBJ)

clean:
	rm -f $(OBJ)
dist: clean
	mkdir -p woyf-$(VERSION)
	cp -R LICENSE Makefile README.md woyf.1 $(SRC) config.mk\
		woyf-$(VERSION)
	tar -cf - woyf-$(VERSION) | gzip > woyf-$(VERSION).tar.gz
	rm -rf woyf-$(VERSION)
install: woyf
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f woyf $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/woyf
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < woyf.1 > $(DESTDIR)$(MANPREFIX)/man1/woyf.1
	chmod 644 $(DESTDIR)$(PREFIX)/man1/woyf.1
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/st
	rm -f $(DESTDIR)$(MANPREFIX)/man1/st.1

.PHONY: all clean dist install uninstall
