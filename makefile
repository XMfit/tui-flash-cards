# Compiler
CC = gcc
# Libraries
LIB = -lsqlite3
# Targets
all: flash-cards

flash-cards: main.c db.c
			$(CC) main.c db.c -o flashcards $(LIB)

clean:
			rm -rf flash-cards
