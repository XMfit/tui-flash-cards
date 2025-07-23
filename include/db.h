#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_BUFFER 1024

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

typedef struct {
   int id;
   char* name;
   int card_count;
} DeckInfo;

typedef struct {
   DeckInfo* items;
   size_t count;
   size_t capacity;
} DeckInfoList;

/*
* Brief - Create sqlite database
* Input - Reference to sqlite3 pointer
*/
void setup_database(sqlite3** db);

/*
* Brief - Verify if deck exists by deck_name, and sets deck id if passed
* Input - sqlite3* db, deck_name, and pointer to deck id
* Output - Flag specifying if deck exists or not
*/
int deck_exists(sqlite3* db, const char* deck_name, int* deck_id);

/*
* Brief - Load all deck information into DeckInfoList structure 
* Input - database pointer, DeckInfoList struct pointer
*/ 
void load_deck_list(sqlite3* db, DeckInfoList* list);

/*
* Brief - free DeckInfoList
*/
void free_deck_list(DeckInfoList* list);

/*
* Brief - Load cards into deck by id
* Input - Database pointer, deck_id, and a pointer to the Deck structure
*/
void load_deck_cards(sqlite3* db, int deck_id, Deck* deck);
void free_deck_cards(Deck* deck);

void create_deck(sqlite3* db, char* deck_name);
void delete_deck_by_name(sqlite3* db, char* deck_name);
void delete_deck_by_id(sqlite3* db, int deck_id);

void add_card(sqlite3* db, int deck_id, const char* front, const char* back);
void delete_card_by_id(sqlite3* db, int card_id);

// extra util functions
void remove_newline(char* str);
void to_lowercase(char* str);

#endif

