#include "../include/db.h"
#include "../include/tui.h"

#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define DB_RELATIVE_PATH "tui-cards/flashcards.db"

void setup_database(sqlite3 **db) {
   char* err_msg = 0;
   const char* home = getenv("HOME");
   if (!home) {
      fprintf(stderr, "Could not determine $HOME\n");
      exit(EXIT_FAILURE);
   }

   char db_path[PATH_MAX];
   snprintf(db_path, sizeof(db_path), "%s/%s", home, DB_RELATIVE_PATH);

   char dir_path[PATH_MAX];
   snprintf(dir_path, sizeof(dir_path), "%s/tui-cards", home);

   if (access(dir_path, F_OK) != 0) {
      if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) {
         perror("Failed to create ~/tui-cards directory");
         exit(EXIT_FAILURE);
      }
   }

   int rc = sqlite3_open(db_path, db);
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

void load_deck_list(sqlite3* db, DeckInfoList* list) {
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

void free_deck_list(DeckInfoList* list) {
    for (int i = 0; i < list->count; ++i) {
        free(list->items[i].name);  // Free the strdup'd name
    }
    free(list->items);  // Free the array itself
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void load_deck_cards(sqlite3* db, int deck_id, Deck* deck) {
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

   char status_msg[MAX_BUFFER] = {0};

   // Get deck name
   sqlite3_stmt* name_stmt;
   const char* name_sql = "SELECT name FROM decks WHERE id = ?";
   if (sqlite3_prepare_v2(db, name_sql, -1, &name_stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Failed to prepare name statement: %s", sqlite3_errmsg(db));
      perrorw(status_msg);
      return;
   }

   sqlite3_bind_int(name_stmt, 1, deck_id);
   if (sqlite3_step(name_stmt) == SQLITE_ROW) {
      const unsigned char* name = sqlite3_column_text(name_stmt, 0);
      deck->deck_name = strdup((const char*)name);
   } else {
      snprintf(status_msg, sizeof(status_msg), "Deck ID %d not found", deck_id);
      perrorw(status_msg);
      sqlite3_finalize(name_stmt);
      return;
   }
   sqlite3_finalize(name_stmt);

   // Get cards
   sqlite3_stmt* stmt;
   const char* sql = "SELECT id, front, back FROM cards WHERE deck_id = ?";

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Failed to prepare card statement: %s", sqlite3_errmsg(db));
      perrorw(status_msg);
      return;
   }

   sqlite3_bind_int(stmt, 1, deck_id);
   
   while (sqlite3_step(stmt) == SQLITE_ROW) {
      int id = sqlite3_column_int(stmt, 0);
      const unsigned char* front = sqlite3_column_text(stmt, 1);
      const unsigned char* back = sqlite3_column_text(stmt, 2);

      Cards card = {
         .id = id,
         .deck_id = deck_id,
         .study_flag = 0,
         .front = strdup((const char*)front),
         .back = strdup((const char*)back)
      };
      da_append(deck, card);
   }

   sqlite3_finalize(stmt);
}

void reset_study_flags(Deck* deck) {
   for (size_t i = 0; i < deck->count; i++) {
      deck->items[i].study_flag = 0;
   }
}

void free_deck_cards(Deck* deck) {
   free(deck->deck_name);
   for (size_t i = 0; i < deck->count; i++) {
      free(deck->items[i].front);
      free(deck->items[i].back);
   }
   free(deck->items);
}

void create_deck(sqlite3 *db, char* deck_name) {
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

void delete_deck_by_name(sqlite3* db, char* deck_name) {
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

void delete_deck_by_id(sqlite3* db, int deck_id) {
   char status_msg[MAX_BUFFER] = {0};
   sqlite3_stmt* stmt;

   const char *delete_sql = "DELETE FROM decks WHERE id = ?;";
   if (sqlite3_prepare_v2(db, delete_sql, -1, &stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Deck deletion failed [id=%d]: %s", deck_id, sqlite3_errmsg(db));
      perrorw(status_msg);
      return;
   }

   sqlite3_bind_int(stmt, 1, deck_id);
   if (sqlite3_step(stmt) == SQLITE_DONE) {
      snprintf(status_msg, sizeof(status_msg), "Deck [id=%d] and its cards have been deleted", deck_id);
      popup_message(stdscr, status_msg);
   } else {
      snprintf(status_msg, sizeof(status_msg), "Deck deletion failed [id=%d]: %s", deck_id, sqlite3_errmsg(db));
      perrorw(status_msg);
   }

   sqlite3_finalize(stmt);
}

void add_card(sqlite3* db, int deck_id, const char* front, const char* back) {
   char status_msg[MAX_BUFFER] = {0};
   sqlite3_stmt* stmt;

   const char *insert_sql = "INSERT INTO cards (deck_id, front, back) VALUES (?, ?, ?);";
   if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Prepare failed %s", sqlite3_errmsg(db));
      perrorw(status_msg);
      return;
   }

   sqlite3_bind_int(stmt, 1, deck_id);
   sqlite3_bind_text(stmt, 2, front, -1, SQLITE_STATIC);
   sqlite3_bind_text(stmt, 3, back, -1, SQLITE_STATIC);

   if (sqlite3_step(stmt) != SQLITE_DONE) {
      snprintf(status_msg, sizeof(status_msg), "Failed to insert card %s", sqlite3_errmsg(db));
      perrorw(status_msg);
   }

   sqlite3_finalize(stmt);
}

void delete_card_by_id(sqlite3* db, int card_id) {
   char status_msg[MAX_BUFFER] = {0};
   sqlite3_stmt* stmt;
   const char* sql = "DELETE FROM cards WHERE id = ?;";

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Prepare failed %s", sqlite3_errmsg(db));
      perrorw(status_msg);
      return;
   }

   sqlite3_bind_int(stmt, 1, card_id);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
      snprintf(status_msg, sizeof(status_msg), "Card Deleted");
      perrorw(status_msg);
   } else {
      snprintf(status_msg, sizeof(status_msg), "Failed to delete card %s", sqlite3_errmsg(db));
      perrorw(status_msg);
   }

   sqlite3_finalize(stmt);
}

void update_card(sqlite3* db, int card_id, const char* new_front, const char* new_back) {
   char status_msg[MAX_BUFFER] = {0};
   sqlite3_stmt* stmt;
   const char* sql = "UPDATE cards SET front = ?, back = ? WHERE id = ?;";

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Prepare failed: %s", sqlite3_errmsg(db));
      perrorw(status_msg);
      return;
   }

   sqlite3_bind_text(stmt, 1, new_front, -1, SQLITE_STATIC);
   sqlite3_bind_text(stmt, 2, new_back, -1, SQLITE_STATIC);
   sqlite3_bind_int(stmt, 3, card_id);

   if (sqlite3_step(stmt) != SQLITE_DONE) {
      snprintf(status_msg, sizeof(status_msg), "Failed to update card: %s", sqlite3_errmsg(db));
      perrorw(status_msg);
   } else {
      snprintf(status_msg, sizeof(status_msg), "Card updated.");
      perrorw(status_msg);
   }

   sqlite3_finalize(stmt);
}

int deck_exists(sqlite3* db, const char* deck_name, int* deck_id) {
   char status_msg[MAX_BUFFER] = {0};
   sqlite3_stmt* stmt;
   const char* sql = "SELECT id FROM decks WHERE name = ? LIMIT 1;";

   if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
      snprintf(status_msg, sizeof(status_msg), "Prepare failed %s", sqlite3_errmsg(db));
      perrorw(status_msg);
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

