#NAME: Keyu Ji
#EMAIL: g.jiekyu@gmail.com
#ID: 704966744

CC = gcc
CFLAGS = -Wall -Wextra -g

simpsh: lab1.c
	$(CC) $(CFLAGS) lab1.c -o $@

check: lab1
	./test.sh

dist: lab1-704966744.tar.gz
sources = Makefile lab1.c README test.sh
lab1-704966744.tar.gz: $(sources)
	tar -cvzf $@ $(sources)

clean:
	rm -f simpsh *.tar.gz