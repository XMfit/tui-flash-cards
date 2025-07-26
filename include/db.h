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
   int study_flag;
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

/*
* Brief - Reset the study flag on cards in a deck 
* Input - Deck structure
*/
void reset_study_flags(Deck* deck);

/*
* Brief - Free deck from memory 
* Input - Deck structure 
*/
void free_deck_cards(Deck* deck);

/*
* Brief - Add deck to deck table in database 
* Input - Database pointer, and name of deck for table 
*/
void create_deck(sqlite3* db, char* deck_name);

/*
* Brief - Delete deck from table via its name 
* Input - Database pointer, and name of deck to delete 
*/
void delete_deck_by_name(sqlite3* db, char* deck_name);

/*
* Brief - Delete deck from table via its ID 
* Input - Database pointer, and id of deck 
*/
void delete_deck_by_id(sqlite3* db, int deck_id);

/*
* Brief - Add card to the card table in the database 
* Input - Database pointer, id of deck, front and back text of card 
*/
void add_card(sqlite3* db, int deck_id, const char* front, const char* back);

/*
* Brief - Delete card from database by ID 
* Input - Database pointer, id of card
*/
void delete_card_by_id(sqlite3* db, int card_id);

/*
* Brief - Update card in the database 
* Input - Database pointer, id of card, front and back text of card
*/
void update_card(sqlite3* db, int card_id, const char* new_front, const char* new_back);

// extra util functions
void remove_newline(char* str);
void to_lowercase(char* str);

#endif

