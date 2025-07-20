#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define NAME_BUFFER 1024

// Yoinked from Tsoding
// Dyanmic Arrays in C
#define da_append(xs, x)                                                             \
    do {                                                                             \
        if ((xs)->count >= (xs)->capacity) {                                         \
            if ((xs)->capacity == 0) (xs)->capacity = 256;                           \
            else (xs)->capacity *= 2;                                                \
            (xs)->items = realloc((xs)->items, (xs)->capacity*sizeof(*(xs)->items)); \
        }                                                                            \
                                                                                     \
        (xs)->items[(xs)->count++] = (x);                                            \
    } while (0)

typedef struct {
   int id;
   int deck_id;
   char *front;
   char *back;
} Cards;

typedef struct {
   char* deck_name;
   Cards *items;
   size_t count;
   size_t capacity;
} Deck;

void setup_database(sqlite3** db);
int deck_exists(sqlite3* db, const char* deck_name, int* deck_id);
void list_decks(sqlite3* db);
int load_deck(sqlite3* db, const char* deck_name, Deck* deck);

void create_deck(sqlite3* db);
void delete_deck(sqlite3* db);
void add_card(sqlite3* db, const char* deck_name);
void delete_card(sqlite3* db, int card_id);

// extra util functions
void remove_newline(char* str);
void to_lowercase(char* str);

#endif

