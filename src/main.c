#include <linux/limits.h>
#include <ncurses.h>
#include <sqlite3.h>
#include "../include/db.h"
#include "../include/tui.h"

extern const char* main_menu_choices[];
extern const char* deck_actions_menu_choices[];

int main() {
   // Set up DB
   sqlite3* db;
   setup_database(&db);

   // Set up screen
   initscr();
   set_escdelay(25);
   cbreak();
   noecho();
   keypad(stdscr, TRUE);
   curs_set(0);
   
   // Main window dim
   int height, width;
   height = MAIN_MENU_HEIGHT;
   width = MAIN_MENU_WIDTH;
   int starty = (LINES - height) / 2;
   int startx = (COLS - width) / 2;

   WINDOW* menu_win = newwin(height, width, starty, startx);

   // Main loop for user interaction
   int running = 1;
   while (running) {
      int choice = draw_menu(menu_win, starty, startx, main_menu_choices, 4, "Main Menu");
      char input1[MAX_BUFFER];
      char input2[MAX_BUFFER];
      Deck decks = {0};
      DeckInfoList deck_info = {0};
      switch(choice) {
         case 0: { // create deck
            form_input(stdscr, "Enter deck name: ", input1, MAX_BUFFER);
            if (strlen(input1) == 0) {
               perrorw("Enter valid deck name");
               continue;
            }
            create_deck(db, input1);
            perrorw("Deck added");
            break;
         }
         case 1: { // View deck data and select deck
            load_deck_info_list(db, &deck_info);
            int deck_id = show_deck_info(stdscr, &deck_info);
            draw_menu(menu_win, starty, startx, deck_actions_menu_choices, 6, "Deck Manager");
            break;
         }
         case 2: { // delete deck
            form_input(stdscr, "Enter deck to delete ", input1, MAX_BUFFER);
            if (strlen(input1) == 0) {
               perrorw("Enter valid deck name");
               continue;
            }
            delete_deck(db, input1);
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
