# Compiler
CC = gcc
# Libraries
LIB = -lsqlite3 -lncurses
# Targets
all: bin/flash-cards

bin/flash-cards: src/main.c src/db.c src/tui.c
	$(CC) src/main.c src/db.c src/tui.c -o bin/flash-cards $(LIB)

clean:
	rm -rf bin/flash-cards
