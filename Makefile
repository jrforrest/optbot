CFLAGS+= -Wall
CC=gcc

PREFIX?=/usr/local

ifeq ($(DEBUG), true)
	CFLAGS += -DDEBUG -g
endif

bin/optbot: src/optbot.c
	$(CC) $(CFLAGS) src/optbot.c -loptbot -o bin/optbot
test: test.sh bin/optbot
	sh test.sh
.PHONY: test
install: bin/optbot
	cp -pf  bin/optbot $(PREFIX)/bin/
	cp -pf  doc/optbot.1 $(PREFIX)/share/man/man1/
.PHONY: install
clean:
	rm -f bin/optbot
.PHONY: clean
