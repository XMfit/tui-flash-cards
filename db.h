#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define NAME_BUFFER 1024

void setup_database(sqlite3** db);
void create_deck(sqlite3* db);
void delete_deck(sqlite3* db);
void add_card(sqlite3* db);

void remove_newline(char* str);
void to_lowercase(char* str);

#endif
