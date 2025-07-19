#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

#define NAME_BUFFER 1024

void print_menu();
void remove_newline(char* str);
void to_lowercase(char* str);

void create_deck(sqlite3* db);
void delete_deck(sqlite3* db);

int main() {
   sqlite3* db;
   char* err_msg = 0;

   int rc = sqlite3_open("flashcards.db", &db);
   if (rc != SQLITE_OK) {
      fprintf(stderr, "Unable to open database: %s\n", sqlite3_errmsg(db));
      return 1;
   }
   sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, 0);

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

   if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
      fprintf(stderr, "SQL Error: %s\n", err_msg);
      sqlite3_free(err_msg);
      sqlite3_close(db);
      return 1;
   }

   int status = 1;
   // Repl
   while (status) {
      print_menu();
      scanf("%d", &status);
      int c;
      while ((c = getchar()) != '\n' && c != EOF) { }

      switch (status) {
         case 0:
            break;
         case 1:
            create_deck(db);
            break;
         case 2:
            delete_deck(db);
            break;
         default:
            printf("Invalid Selection\n");
            break;
      }
   }

   sqlite3_close(db);
   return 0;
}

void print_menu() {
   printf("--------------------------\n");
   printf("Options:\n");
   printf("0.) Quit\n1.) Create Deck\n2.) Delete Deck\n");
}

void create_deck(sqlite3 *db) {
   char deck_name[NAME_BUFFER];
   printf("Enter deck name: ");
   fgets(deck_name, sizeof(deck_name), stdin);
   to_lowercase(deck_name);
   remove_newline(deck_name);

   sqlite3_stmt* stmt;

   // Check if deck already exists  
   const char *check_sql = "SELECT id FROM decks WHERE name = ?;";
   if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, 0) != SQLITE_OK) {
      printf("Prepare failed: %s\n", sqlite3_errmsg(db));
      return;
   }

   sqlite3_bind_text(stmt, 1, deck_name, -1, SQLITE_STATIC);

   if (sqlite3_step(stmt) == SQLITE_ROW) {
      int existing_id = sqlite3_column_int(stmt, 0);
      printf("Deck already exists with ID: %d\n", existing_id);
      sqlite3_finalize(stmt);
      return;
   }
   sqlite3_finalize(stmt);

   // Insert new deck
   const char *insert_sql = "INSERT INTO decks (name) VALUES (?);";
   if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0) != SQLITE_OK) {
      printf("Prepare failed: %s\n", sqlite3_errmsg(db));
      return;
   }

   sqlite3_bind_text(stmt, 1, deck_name, -1, SQLITE_STATIC);

   if (sqlite3_step(stmt) == SQLITE_DONE) {
      int new_id = (int)sqlite3_last_insert_rowid(db);
      printf("Deck created: %s (ID: %d)\n", deck_name, new_id);
   } else {
      fprintf(stderr, "Failed to create deck: %s\n", sqlite3_errmsg(db));
   }

   sqlite3_finalize(stmt);
}

void delete_deck(sqlite3* db) {
   char deck_name[NAME_BUFFER];
   printf("Enter deck name to delete: ");
   fgets(deck_name, sizeof(deck_name), stdin);
   to_lowercase(deck_name);
   remove_newline(deck_name);

   sqlite3_stmt* stmt;

   // Find deck ID
   const char *select_sql = "SELECT id FROM decks WHERE name = ?;";
   if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0) != SQLITE_OK) {
      printf("Prepare failed: %s\n", sqlite3_errmsg(db));
      return;
   }

   sqlite3_bind_text(stmt, 1, deck_name, -1, SQLITE_STATIC);
   int rc = sqlite3_step(stmt);
   if (rc != SQLITE_ROW) {
      printf("Deck '%s' does not exist.\n", deck_name);
      sqlite3_finalize(stmt);
      return;
   }

   int deck_id = sqlite3_column_int(stmt, 0);
   sqlite3_finalize(stmt);

   // Delete deck and cascade-delete cards
   const char *delete_sql = "DELETE FROM decks WHERE id = ?;";
   if (sqlite3_prepare_v2(db, delete_sql, -1, &stmt, 0) != SQLITE_OK) {
      printf("Prepare failed: %s\n", sqlite3_errmsg(db));
      return;
   }

   sqlite3_bind_int(stmt, 1, deck_id);
   if (sqlite3_step(stmt) == SQLITE_DONE) {
      printf("Deck '%s' (ID: %d) and its cards have been deleted.\n", deck_name, deck_id);
   } else {
      printf("Failed to delete deck: %s\n", sqlite3_errmsg(db));
   }

   sqlite3_finalize(stmt);
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

