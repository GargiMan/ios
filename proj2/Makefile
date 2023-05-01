# Makefile IOS DU2
# Autor: Marek Gergel, FIT

PROJ = proj2
CC = gcc
CFLAGS = -std=gnu99 -g -O0 -Wall -Wextra -Werror -pedantic -pthread -lrt

.PHONY: zip clean

all: $(PROJ)

$(PROJ): $(PROJ).c
	$(CC) $(CFLAGS) $(PROJ).c -o $@

check:
	@test -f $(PROJ) && ./check-syntax.sh <$(PROJ).out || echo "File '$(PROJ).out' does not exist"

clean:
	rm -f $(PROJ) $(PROJ).zip *.out

zip:
	zip $(PROJ).zip $(PROJ).c Makefile