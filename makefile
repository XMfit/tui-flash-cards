# Compiler
CC = gcc
# Libraries
LIB = -lsqlite3 -lncurses
# Targets
all: bin/flash-cards

bin/flash-cards: src/main.c src/db.c src/tui.c src/menu_utils.c | bin
	$(CC) src/main.c src/db.c src/tui.c src/menu_utils.c -o bin/flash-cards $(LIB)

# Create bin directory if it doesn't exist
bin:
	mkdir -p bin

clean:
	rm -rf bin/flash-cards

