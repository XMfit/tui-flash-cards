#include "../include/tui.h"

const char* main_menu_choices[] = {
   "Create New Deck",
   "View All Deck Names",
   "Select a Deck",
   "Delete a Deck",
   "Exit"
};

// Generic func to draw menu and allow for select an item
int draw_menu(WINDOW* win, int starty, int startx, const char** choices, int n_choices, const char* title) {
   int highlight = 0;
   int choice = -1;
   int ch;
   
   keypad(win, TRUE);
   while (1) {
      werase(win);
      box(win, 0, 0);
      mvwprintw(win, 1, 2, "%s", title);

      for (int i = 0; i < n_choices; i++) {
         if (i == highlight) {
               wattron(win, A_REVERSE);
               mvwprintw(win, i + 3, 4, "%s", choices[i]);
               wattroff(win, A_REVERSE);
         } else {
               mvwprintw(win, i + 3, 4, "%s", choices[i]);
         }
      }

      mvwprintw(win, n_choices + 5, 2, "Arrow keys to navigate, Enter to select, ESC to quit");
      wrefresh(win);

      ch = wgetch(win);
      switch (ch) {
         case KEY_UP:
               highlight = (highlight == 0) ? n_choices - 1 : highlight - 1;
               break;
         case KEY_DOWN:
               highlight = (highlight == n_choices - 1) ? 0 : highlight + 1;
               break;
         case 10:  // Enter
               return highlight;
         case ESC_KEY:
               return -1;
         default:
               break;
        }
    }
}

void form_input(WINDOW* parent_win, const char* form_prompt, char* input, int max_len) {
   int starty, startx;
   int ch;
   int pos = 0;

   // Calculate position: near bottom center of parent window
   int parent_height, parent_width;
   getmaxyx(parent_win, parent_height, parent_width);

   starty = parent_height - FORM_HEIGHT - 2;
   startx = (parent_width - FORM_WIDTH) / 2;

   // Create form window
   WINDOW* form_win = newwin(FORM_HEIGHT, FORM_WIDTH, starty, startx);
   keypad(form_win, TRUE);
   box(form_win, 0, 0);

   mvwprintw(form_win, 1, 2, "%s", form_prompt);
   wrefresh(form_win);

   // Clear input buffer
   memset(input, 0, max_len);

   // Input loop
   while (1) {
      // Show current input
      mvwprintw(form_win, 2, 2, "%-*s", FORM_WIDTH - 4, input);
      wmove(form_win, 2, 2 + pos);
      wrefresh(form_win);

      ch = wgetch(form_win);

      if (ch == '\n' || ch == KEY_ENTER) {
         break;
      } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
         if (pos > 0) {
            pos--;
            input[pos] = '\0';
         }
      } else if (ch >= 32 && ch < 127) {
         // Printable characters
         if (pos < max_len - 1 && pos < FORM_WIDTH - 4) {  // keep input within buffer and visible space
            input[pos++] = ch;
            input[pos] = '\0';
         }
      }
   }

   werase(form_win);
   wrefresh(form_win);
   delwin(form_win);
}

void popup_message(WINDOW * parent_win, const char* message) {
   int starty, startx;
   int ch;

   // Calculate position 
   int parent_height, parent_width;
   getmaxyx(parent_win, parent_height, parent_width);

   starty = parent_height - POPUP_HEIGHT - 2;
   startx = (parent_width - POPUP_WIDTH) / 2;

   WINDOW* popup_win = newwin(POPUP_HEIGHT, POPUP_WIDTH, starty, startx);
   keypad(popup_win, TRUE);
   box(popup_win, 0, 0);

   mvwprintw(popup_win, 1, 2, "%s", message);
   mvwprintw(popup_win, 3, 2, "Press enter to close");
   wrefresh(popup_win);

   while(1) {
      ch = wgetch(popup_win);
      if (ch == '\n' || ch == KEY_ENTER)
         break;
   }

   werase(popup_win);
   wrefresh(popup_win);
   delwin(popup_win);
}

void perrorw(const char* err_msg) {
   attron(A_BOLD);
   mvprintw(LINES - 3, 0, "%s", err_msg);
   attroff(A_BOLD);
   clrtoeol();
   refresh();
}

