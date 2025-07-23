#include "../include/db.h"
#include "../include/tui.h"
#include <ncurses.h>

void setup_database(sqlite3 **db) {
   char* err_msg = 0;
   int rc = sqlite3_open("flashcards.db", db);
   if (rc != SQLITE_OK) {
      fprintf(stderr, "Unable to open database: %s\n", sqlite3_errmsg(*db));
      exit(EXIT_FAILURE);
   }
   sqlite3_exec(*db, "PRAGMA foreign_keys = ON;", 0, 0, 0);

   const char *sql =
        "CREATE TABLE IF NOT EXISTS decks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE);"

        "CREATE TABLE IF NOT EXISTS cards ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "deck_id INTEGER, "
        "front TEXT NOT NULL, "
        "back TEXT NOT NULL, "
        "FOREIGN KEY(deck_id) REFERENCES decks(id) ON DELETE CASCADE);";

   if (sqlite3_exec(*db, sql, 0, 0, &err_msg) != SQLITE_OK) {
      fprintf(stderr, "SQL Error: %s\n", err_msg);
      sqlite3_free(err_msg);
      sqlite3_close(*db);
      exit(EXIT_FAILURE);
   }
}

void load_deck_info_list(sqlite3* db, DeckInfoList* list) {
    const char* sql =
        "SELECT d.id, d.name, COUNT(c.id) AS card_count "
        "FROM decks d "
        "LEFT JOIN cards c ON d.id = c.deck_id "
        "GROUP BY d.id "
        "ORDER BY d.name ASC;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        //fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Zero out list
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name_text = sqlite3_column_text(stmt, 1);
        int card_count = sqlite3_column_int(stmt, 2);

        if (!name_text) continue;

        DeckInfo info = {
            .id = id,
            .name = strdup((const char*)name_text),
            .card_count = card_count
        };

        da_append(list, info);
    }

    sqlite3_finalize(stmt);
}

void free_deck_info_list(DeckInfoList* list) {
    for (int i = 0; i < list->count; ++i) {
        free(list->items[i].name);  // Free the strdup'd name
    }
    free(list->items);  // Free the array itself
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

int load_deck(sqlite3* db, const char* deck_name, Deck* deck) {
   // Clear any memory before rewriting
   free(deck->deck_name);
   for (size_t i = 0; i < deck->count; ++i) {
      free(deck->items[i].front);
      free(deck->items[i].back);
   }
   free(deck->items);
   deck->deck_name = NULL;
   deck->items = NULL;
   deck->count = 0;
   deck->capacity = 0;
   deck->deck_name = strdup(deck_name);

   // Get deck ID
   int deck_id = -1;
   if (!deck_exists(db, deck_name, &deck_id)) {
      return 0;
   }

   sqlite3_stmt* stmt;
   const char* sql = "SELECT id, deck_id, front, back FROM cards WHERE deck_id = ?";

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
      fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
      return 0;
   }

   sqlite3_bind_int(stmt, 1, deck_id);
   
   // Set card info
   while (sqlite3_step(stmt) == SQLITE_ROW) {
      int id = sqlite3_column_int(stmt, 0);
      int deck_id_val = sqlite3_column_int(stmt, 1);
      const unsigned char* front = sqlite3_column_text(stmt, 2);
      const unsigned char* back = sqlite3_column_text(stmt, 3);

      Cards card = {
         .id = id,
         .deck_id = deck_id_val,
         .front = strdup((const char*)front),
         .back = strdup((const char*)back)
      };
      da_append(deck, card);
   }

   sqlite3_finalize(stmt);
      return 1;
}

void list_decks(sqlite3* db) {
   sqlite3_stmt* stmt;
   const char* sql = "SELECT name FROM decks ORDER BY name ASC;";

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
      fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
      return;
   }

   printf("\nDecks:\n");
   int found = 0;
   while (sqlite3_step(stmt) == SQLITE_ROW) {
      const unsigned char* name = sqlite3_column_text(stmt, 0);
      printf("- %s\n", name);
      found = 1;
   }
   printf("\n");

   if (!found) {
      printf("No decks found.\n");
   }

   sqlite3_finalize(stmt);
}

void create_deck(sqlite3 *db, char* deck_name) {
   to_lowercase(deck_name);
   remove_newline(deck_name);

   char status_msg[MAX_BUFFER] = {0};
   sqlite3_stmt* stmt;

   if (deck_exists(db, deck_name, NULL)) { // return if deck already exists
      snprintf(status_msg, sizeof(status_msg), "Deck: '%s' already exists", deck_name); 
      popup_message(stdscr, status_msg);
      return;
   }

   // Insert new deck
   const char *insert_sql = "INSERT INTO decks (name) VALUES (?);";
   if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Deck creation failed: '%s' - %s", deck_name, sqlite3_errmsg(db));
      perrorw(status_msg);
      return;
   }
   sqlite3_bind_text(stmt, 1, deck_name, -1, SQLITE_STATIC);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
      snprintf(status_msg, sizeof(status_msg), "Deck: '%s' created!", deck_name); 
      popup_message(stdscr, status_msg);
   } else {
      snprintf(status_msg, sizeof(status_msg), "Deck creation failed: '%s' - %s", deck_name, sqlite3_errmsg(db));
      perrorw(status_msg);
   }


   sqlite3_finalize(stmt);
}

void delete_deck(sqlite3* db, char* deck_name) {
   to_lowercase(deck_name);
   remove_newline(deck_name);

   char status_msg[MAX_BUFFER] = {0};
   sqlite3_stmt* stmt;
   int deck_id;

   if (!deck_exists(db, deck_name, &deck_id)) {
      snprintf(status_msg, sizeof(status_msg), "Deck: '%s' does not exist", deck_name);
      popup_message(stdscr, status_msg);
      return;
   }

   // Delete deck and cascade-delete cards
   const char *delete_sql = "DELETE FROM decks WHERE id = ?;";
   if (sqlite3_prepare_v2(db, delete_sql, -1, &stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Deck deletion failed: '%s' - %s", deck_name, sqlite3_errmsg(db));
      perrorw(status_msg);
      return;
   }

   sqlite3_bind_int(stmt, 1, deck_id);
   if (sqlite3_step(stmt) == SQLITE_DONE) {
      snprintf(status_msg, sizeof(status_msg), "Deck: '%s' and its cards have been deleted", deck_name);
      popup_message(stdscr, status_msg);
   } else {
      snprintf(status_msg, sizeof(status_msg), "Deck deletion failed: '%s' - %s", deck_name, sqlite3_errmsg(db));
      perrorw(status_msg);
   }

   sqlite3_finalize(stmt);
}

void add_card(sqlite3* db, const char* deck_name) {
   char front[MAX_BUFFER];
   char back[MAX_BUFFER];

   sqlite3_stmt* stmt;
   int deck_id = 0;

   deck_exists(db, deck_name, &deck_id);

   // Contents of card
   printf("Enter card front: ");
   fgets(front, sizeof(front), stdin);
   remove_newline(front);

   printf("Enter card back: ");
   fgets(back, sizeof(back), stdin);
   remove_newline(back);

   // Insert card
   const char *insert_sql = "INSERT INTO cards (deck_id, front, back) VALUES (?, ?, ?);";
   if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0) != SQLITE_OK) {
      printf("Prepare failed: %s\n", sqlite3_errmsg(db));
      return;
   }

   sqlite3_bind_int(stmt, 1, deck_id);
   sqlite3_bind_text(stmt, 2, front, -1, SQLITE_STATIC);
   sqlite3_bind_text(stmt, 3, back, -1, SQLITE_STATIC);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
      printf("Card added to deck '%s'.\n", deck_name);
   } else {
      printf("Failed to insert card: %s\n", sqlite3_errmsg(db));
   }

   sqlite3_finalize(stmt);
}

void delete_card(sqlite3* db, int card_id) {
   sqlite3_stmt* stmt;
   const char* sql = "DELETE FROM cards WHERE id = ?;";

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
      //fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
      return;
   }

   sqlite3_bind_int(stmt, 1, card_id);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
      //printf("Card with ID %d has been deleted.\n", card_id);
   } else {
      //fprintf(stderr, "Failed to delete card: %s\n", sqlite3_errmsg(db));
   }

   sqlite3_finalize(stmt);
}

int deck_exists(sqlite3* db, const char* deck_name, int* deck_id) {
   sqlite3_stmt* stmt;
   const char* sql = "SELECT id FROM decks WHERE name = ? LIMIT 1;";

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
      //fprintf(stderr, "Prepare failed: %s\n", sqlite3_errmsg(db));
      return 0;
   }

   sqlite3_bind_text(stmt, 1, deck_name, -1, SQLITE_STATIC);

   int exists = 0;
   if (sqlite3_step(stmt) == SQLITE_ROW) {
      if (deck_id) {
         *deck_id = sqlite3_column_int(stmt, 0);
      }
      exists = 1;
   }

   sqlite3_finalize(stmt);
   return exists;
}

void remove_newline(char* str) {
   size_t len = strlen(str);
   if (len > 0 && str[len - 1] == '\n')
      str[len - 1] = '\0';
}

void to_lowercase(char* str) {
   while (*str) {
      *str = tolower((unsigned char)* str);
      str++;
   }
}

