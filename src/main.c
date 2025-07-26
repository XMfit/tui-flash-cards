#include "../include/db.h"
#include "../include/tui.h"
#include <ncurses.h>

void deck_wizard(WINDOW* deck_win, const int deck_id);

extern const char* main_menu_choices[];
extern const char* deck_actions_menu_choices[];

sqlite3* db;

int main() {
   // Set up DB
   setup_database(&db);

   // Set up screen
   initscr();
   set_escdelay(10);
   cbreak();
   noecho();
   keypad(stdscr, TRUE);
   curs_set(0);
   
   // Main window dim
   int height, width;
   height = MAIN_MENU_HEIGHT;
   width = MAIN_MENU_WIDTH;

   WINDOW* menu_win = newwin(height, width, MAIN_MENU_Y, MAIN_MENU_X);

   // Main loop for user interaction
   int running = 1;
   while (running) {
      int choice = draw_menu(menu_win, MAIN_MENU_Y, MAIN_MENU_X, main_menu_choices, 4, "Main Menu");
      char input1[MAX_BUFFER];
      char input2[MAX_BUFFER];
      Deck decks = {0};
      DeckInfoList deck_info = {0};
      switch(choice) {
         case 0: { // create deck
            form_input(stdscr, DECKC_PROMPT, input1, MAX_BUFFER, 1);
            if (strlen(input1) == 0) {
               perrorw("Enter valid deck name");
               continue;
            }
            create_deck(db, input1);
            perrorw("Deck added");
            break;
         }
         case 1: { // View deck data and select deck
            load_deck_list(db, &deck_info);
            if (deck_info.count == 0) {
               popup_message(stdscr, "No Decks Available!");
               free_deck_list(&deck_info);
               break;
            }
            int deck_id = show_deck_info(stdscr, &deck_info);
            free_deck_list(&deck_info);
            if (deck_id > 0)
               deck_wizard(menu_win, deck_id);
            break;
         }
         case 2: { // delete deck
            form_input(stdscr, DECKD_PROMPT, input1, MAX_BUFFER, 1);
            if (strlen(input1) == 0) {
               perrorw("Enter valid deck name");
               continue;
            }
            delete_deck_by_name(db, input1);
            perrorw("Deck deleted");
            break;
         }
         case 3: // exit
         case -1:
            running = 0;
            break;
         default:
            break;
      }
      werase(menu_win);
   }
   delwin(menu_win);
   endwin();
   sqlite3_close(db);
   return 0;
}

void deck_wizard(WINDOW* deck_win, const int deck_id) {
   int running = 1;
   Deck deck = {0};
   load_deck_cards(db, deck_id, &deck);
   char title[MAX_BUFFER];
   snprintf(title, MAX_BUFFER, "Deck Manager - %s", deck.deck_name);

   while (running) {
      int choice = draw_menu(deck_win, MAIN_MENU_Y, MAIN_MENU_X, deck_actions_menu_choices, 5, title);
      load_deck_cards(db, deck_id, &deck);
      char input1[MAX_BUFFER];
      char input2[MAX_BUFFER];
      switch(choice) {
         case 0: { // study deck
            study_cards(stdscr, &deck);
            break;
         }
         case 1: { // view cards
            display_cards(stdscr, &deck);
            break;
         }
         case 2: { // add cards
            card_input(stdscr, CARD_PROMPT, input1, input2, MAX_BUFFER);
             if (strlen(input1) == 0 || strlen(input2) == 0) {
               perrorw("Card information cannot be blank");
               continue;
            }
            add_card(db, deck_id, input1, input2);
            break;
         }
         case 3: { // delete deck
            delete_deck_by_id(db, deck_id);
            perrorw("Deck deleted");
         }
         case 4: // main menu 
         case -1:
            running = 0;
            break;
         default:
            break;
      }
   }
   free_deck_cards(&deck);
}

