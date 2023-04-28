# Makefile IOS DU2
# Autor: Marek Gergel, FIT

PROJ = proj2
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread -lrt

.PHONY: zip clean

all: $(PROJ)

$(PROJ): $(PROJ).c
	$(CC) $(CFLAGS) $(PROJ).c -o $@

zip:
	zip $(PROJ).zip *.c *.h Makefile

clean:
	$(RM) -f $(PROJ) *.out