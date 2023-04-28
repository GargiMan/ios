# Makefile IOS DU2
# Autor: Marek Gergel, FIT

CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic

.PHONY: zip clean

all: proj2

proj2: proj2.c
	$(CC) $(CFLAGS) proj2.c -o $@

zip: *.c Makefile
	zip proj2.zip *.c Makefile

clean:
	rm -f proj2