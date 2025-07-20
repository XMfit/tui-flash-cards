#include <stdio.h>
#include <sqlite3.h>
#include "db.h"

#define NAME_BUFFER 1024

void main_menu();
void deck_manager_menu();
void deck_manager();

sqlite3* db;

int main() {
   setup_database(&db);
   int status = 1;
   // Repl
   while (status) {
      main_menu();
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
            list_decks(db);
            break;
         case 3:
            // Select a deck
            deck_manager();
            break;
         case 4:
            // Delete a deck
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

void deck_manager() {
   char deck_name[NAME_BUFFER];
   printf("Enter deck name: ");
   fgets(deck_name, sizeof(deck_name), stdin);
   to_lowercase(deck_name);
   remove_newline(deck_name);

   if (!deck_exists(db, deck_name, NULL)) {
      printf("Deck does not exist\n");
      return;
   }

   Deck xs = {0};
   load_deck(db, deck_name, &xs);

   int status = 1;
   while (status) {
      deck_manager_menu();
      scanf("%d", &status);
      int c;
      while ((c = getchar()) != '\n' && c != EOF) { }
      switch(status) {
         case 0:
            break;
         case 1: {
            // View Cards 
            for (size_t i = 0; i < xs.count; i++) {
               printf("Card %zu:\nFront: %s\nBack: %s\n", i + 1, xs.items[i].front, xs.items[i].back);
               printf("id: %d, deck_id: %d\n", xs.items[i].id, xs.items[i].deck_id);
               printf("--------------------------\n");
            }
            break;
         }
         case 2:
            // Add Cards 
            add_card(db, deck_name);
            load_deck(db, deck_name, &xs);
            break;
         case 3: {
            // Delete card
            int card_id = 0;
            printf("Input card ID: ");
            scanf("%d", &card_id);
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            delete_card(db, card_id);
            load_deck(db, deck_name, &xs);
         }
         default:
            printf("Invalid Selection\n");
            break;
      }
   }
   free(xs.deck_name);
   for (size_t i = 0; i < xs.count; i++) {
      free(xs.items[i].front);
      free(xs.items[i].back);
   }
   free(xs.items);
}

void main_menu() {
   printf("Main Menu\n");
   printf("--------------------------\n");
   printf("Options:\n");
   printf("0.) Quit\n1.) Create New Deck\n2.) View All Deck Names\n3.) Select a Deck\n4.) Delete a Deck\n");
}

void deck_manager_menu() {
   printf("Deck Manager\n");
   printf("--------------------------\n");
   printf("Options:\n");
   printf("0.) Quit\n1.) View Cards in Deck\n2.) Add card to deck\n3.) Delete Card\n");
}

