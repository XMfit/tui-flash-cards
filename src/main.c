#include <ncurses.h>
#include <sqlite3.h>
#include "../include/db.h"
#include "../include/tui.h"

extern const char* main_menu_choices[];

int main() {
   // Set up DB
   sqlite3* db;
   setup_database(&db);

   // Set up screen
   initscr();
   cbreak();
   noecho();
   keypad(stdscr, TRUE);
   curs_set(0);
   
   // Main window dim
   int height, width;
   height = 15;
   width = 70;
   int starty = (LINES - height) / 2;
   int startx = (COLS - width) / 2;

   WINDOW* menu_win = newwin(height, width, starty, startx);

   // Main loop for user interaction
   int running = 1;
   while (running) {
      int choice = draw_menu(menu_win, starty, startx, main_menu_choices, 5, "Main Menu");
      char user_input[MAX_BUFFER];
      switch(choice) {
         case 0: {
            form_input(stdscr, "Enter deck name: ", user_input, MAX_BUFFER);
            attron(A_BOLD);
            mvprintw(LINES - 3, 0, "Adding Deck: %s", user_input);
            attroff(A_BOLD);
            popup_message(stdscr, "AWOOGA");
            clrtoeol();
            refresh();
            break;
         }
         case 1:
            break;
         case 2:
            break;
         case 3:
            break;
         case 4:
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
