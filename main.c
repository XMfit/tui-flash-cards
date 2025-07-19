#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include "db.h"

#define NAME_BUFFER 1024

void print_menu();
void remove_newline(char* str);
void to_lowercase(char* str);

void create_deck(sqlite3* db);
void delete_deck(sqlite3* db);
void add_card(sqlite3* db);

int main() {
   sqlite3* db;
   setup_database(&db);

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
         case 3:
            add_card(db);
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
   printf("0.) Quit\n1.) Create Deck\n2.) Delete Deck\n3.) Add Card\n");
}

