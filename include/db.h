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
* Brief - Initialize and create SQLite database connection
* Input - Pointer to sqlite3* database handle (output parameter)
* Output - None (assumes error handling inside)
*/
void setup_database(sqlite3** db);

/*
* Brief - Check if a deck exists by name and optionally retrieve its ID
* Input - db: SQLite database handle
*         deck_name: name of the deck to check
*         deck_id: pointer to int to store deck ID if found (can be NULL)
* Output - Returns 1 if deck exists, 0 otherwise
*/
int deck_exists(sqlite3* db, const char* deck_name, int* deck_id);

/*
* Brief - Load all deck metadata into a DeckInfoList structure
* Input - db: SQLite database handle
*         list: pointer to DeckInfoList struct to populate
* Output - None (assumes list is initialized/empty)
*/
void load_deck_list(sqlite3* db, DeckInfoList* list);

/*
* Brief - Free memory allocated inside a DeckInfoList structure
* Input - list: pointer to DeckInfoList to free
* Output - None
*/
void free_deck_list(DeckInfoList* list);

/*
* Brief - Load all cards for a given deck ID into a Deck structure
* Input - db: SQLite database handle
*         deck_id: ID of the deck whose cards to load
*         deck: pointer to Deck struct to populate
* Output - None
*/
void load_deck_cards(sqlite3* db, int deck_id, Deck* deck);

/*
* Brief - Reset the study_flag on all cards in the Deck to 0 (not studied)
* Input - deck: pointer to Deck whose cards will be reset
* Output - None
*/
void reset_study_flags(Deck* deck);

/*
* Brief - Free all dynamically allocated memory inside a Deck structure
* Input - deck: pointer to Deck to free
* Output - None
*/
void free_deck_cards(Deck* deck);

/*
* Brief - Insert a new deck into the database
* Input - db: SQLite database handle
*         deck_name: name of the deck to create
* Output - None
*/
void create_deck(sqlite3* db, char* deck_name);

/*
* Brief - Delete a deck and its associated cards by deck name
* Input - db: SQLite database handle
*         deck_name: name of the deck to delete
* Output - None
*/
void delete_deck_by_name(sqlite3* db, char* deck_name);

/*
* Brief - Delete a deck and its associated cards by deck ID
* Input - db: SQLite database handle
*         deck_id: ID of the deck to delete
* Output - None
*/
void delete_deck_by_id(sqlite3* db, int deck_id);

/*
* Brief - Add a card to the database under a specific deck
* Input - db: SQLite database handle
*         deck_id: ID of the deck to add card to
*         front: front text of the card
*         back: back text of the card
* Output - None
*/
void add_card(sqlite3* db, int deck_id, const char* front, const char* back);

/*
* Brief - Delete a card from the database by card ID
* Input - db: SQLite database handle
*         card_id: ID of the card to delete
* Output - None
*/
void delete_card_by_id(sqlite3* db, int card_id);

/*
* Brief - Update the front and back text of a card by card ID
* Input - db: SQLite database handle
*         card_id: ID of the card to update
*         new_front: new front text
*         new_back: new back text
* Output - None
*/
void update_card(sqlite3* db, int card_id, const char* new_front, const char* new_back);

/*
* Brief - Remove newline character from a string, if present
* Input - str: string to sanitize (modified in place)
* Output - None
*/
void remove_newline(char* str);

/*
* Brief - Convert a string to lowercase in-place
* Input - str: string to convert
* Output - None
*/
void to_lowercase(char* str);

#endif
