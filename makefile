# Compiler 
CC = gcc

# Source Files 
SRCS = src/main.c src/db.c src/tui.c src/menu_utils.c

# Libraries
LIBS = -lsqlite3 -lncurses

# Output binary 
TARGET = bin/flash-cards 

all: $(TARGET)

$(TARGET): $(SRCS) | bin
	$(CC) -o $@ $^ $(LIBS)

# Create bin directory if it doesn't exist
bin:
	mkdir -p bin

clean:
	rm -rf $(TARGET)

