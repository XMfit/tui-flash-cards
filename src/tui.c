#include "../include/tui.h"

const char* main_menu_choices[] = {
   "Create New Deck",
   "Select a Deck to Study or Edit",
   "Delete a Deck",
   "Exit"
};

const char* deck_actions_menu_choices[] = {
   "Study This Deck",
   "View Cards",
   "Add Card",
   "Edit Card",
   "Delete Card",
   "Delete Deck",
   "Back to Main Menu"
};

// I'd like to rework these two functions to have some sort of helper function for the selections
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

int show_deck_info(WINDOW* parent, DeckInfoList* info) {
   int highlight = 0;
   int choice = -1;
   int ch;

   int max_width = calc_max_line_width(info) + GENERIC_PADDING;
   int height = info->count + GENERIC_PADDING;

   WINDOW* win = create_centered_window(parent, height, max_width);
   keypad(win, TRUE);

   while(1) {
      werase(win);
      box(win, 0, 0);
      mvwprintw(win, 1, 2, "Deck Info");

      for (int i = 0; i < info->count; i++) {
         if (i == highlight) {
            wattron(win, A_REVERSE);
            mvwprintw(win, i + 2, 2, "%d: %s (%d cards)",
                      info->items[i].id,
                      info->items[i].name,
                      info->items[i].card_count);
            wattroff(win, A_REVERSE);
         } else { 
            mvwprintw(win, i + 2, 2, "%d: %s (%d cards)",
                      info->items[i].id,
                      info->items[i].name,
                      info->items[i].card_count);
         }
      }
      mvwprintw(win, info->count + 5, 2, "Arrow keys to navigate, Enter to select, ESC to quit");
      wrefresh(win);

      ch = wgetch(win);
      switch (ch) {
         case KEY_UP:
            highlight = (highlight == 0) ? info->count - 1 : highlight - 1;   
            break;
         case KEY_DOWN:
            highlight = (highlight == info->count - 1) ? 0 : highlight + 1;
            break;
         case 10:  // Enter
            clear_and_destroy_window(win);
            return info->items[highlight].id;
         case ESC_KEY:
            clear_and_destroy_window(win);
            return -1;
         default:
            break;
        }
   }
}

void form_input(WINDOW* parent_win, const char* form_prompt, char* input, int max_len) {
   WINDOW* form_win = create_centered_window(parent_win, FORM_HEIGHT, FORM_WIDTH);

   mvwprintw(form_win, 1, 2, "%s", form_prompt);
   wrefresh(form_win);

   if (!get_input_line(form_win, 2, 2, input, max_len, FORM_WIDTH - VISIBLE_WIDTH_MARGIN, 1)) {
      memset(input, 0, max_len);
   }

   clear_and_destroy_window(form_win);
}

void card_input(WINDOW* parent_win, const char* form_prompt, char* input1, char* input2, int max_len) {
   WINDOW* card_win = create_centered_window(parent_win, CARD_FORM_HEIGHT, CARD_FORM_WIDTH);

   mvwprintw(card_win, 1, 2, "%s", form_prompt);
   mvwprintw(card_win, 2, 2, "Input 1:");
   wrefresh(card_win);

   if (!get_input_line(card_win, 3, 2, input1, max_len, CARD_FORM_WIDTH - VISIBLE_WIDTH_MARGIN, 0)) {
      memset(input1, 0, max_len);
      memset(input2, 0, max_len);
      clear_and_destroy_window(card_win);
      return;
   }

   mvwprintw(card_win, 4, 2, "Input 2:");
   wrefresh(card_win);

   if (!get_input_line(card_win, 5, 2, input2, max_len, CARD_FORM_WIDTH - VISIBLE_WIDTH_MARGIN, 0)) {
      memset(input1, 0, max_len);
      memset(input2, 0, max_len);
      clear_and_destroy_window(card_win);
      return;
   }
   clear_and_destroy_window(card_win);
}

void popup_message(WINDOW * parent_win, const char* message) {
   WINDOW* popup_win = create_centered_window(parent_win, POPUP_HEIGHT, POPUP_WIDTH);
   int ch;

   mvwprintw(popup_win, 1, 2, "%s", message);
   mvwprintw(popup_win, 3, 2, "Press enter to close");
   wrefresh(popup_win);

   while(1) { // Close popup when enter is pressed
      ch = wgetch(popup_win);
      if (ch == '\n' || ch == KEY_ENTER)
         break;
   }
   clear_and_destroy_window(popup_win);
}

// Helper functions 
int get_input_line(WINDOW* win, int y, int x, char* buffer, int max_len, int visible_width, int dash_flag) {
   int len = 0;
   int cursor = 0;
   int ch;

   keypad(win, TRUE);
   curs_set(1);
   memset(buffer, 0, max_len);

   while (1) {
      // Draw input line
      mvwprintw(win, y, x, "%-*s", visible_width, buffer);
      wmove(win, y, x + cursor);
      wrefresh(win);

      ch = wgetch(win);

      if (ch == '\n' || ch == KEY_ENTER) {
         curs_set(0);
         return 1;  // submitted
      } else if (ch == KEY_LEFT) { // move cursor position if boundaries are okay
         if (cursor > 0)
            cursor--;
      } else if (ch == KEY_RIGHT) {
         if (cursor < len)
            cursor++;
      } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
         if (cursor > 0) { // move mem back one and reduce len and cursor
            memmove(&buffer[cursor - 1], &buffer[cursor], len - cursor + 1);
            cursor--;
            len--;
         }
      } else if (ch >= 32 && ch < 127) { // mem move forward one and increase len and cursor
         if (len < max_len - 1 && len < visible_width - 1) {
            memmove(&buffer[cursor + 1], &buffer[cursor], len - cursor + 1);
            if(dash_flag)
               buffer[cursor] = (ch == 32) ? '-' : ch;
            else
               buffer[cursor] = ch;
            cursor++;
            len++;
         }
      } else if (ch == ESC_KEY) {
         buffer[0] = '\0';
         curs_set(0);
         return 0;  // cancelled
      }
   }
}


WINDOW* create_centered_window(WINDOW* parent, int height, int width) {
   // Print centered under or in parent window
   int parent_height, parent_width;
   getmaxyx(parent, parent_height, parent_width);

   int starty = parent_height - height - BOTTOM_MARGIN;
   int startx = (parent_width - width) / 2;

   WINDOW* win = newwin(height, width, starty, startx);
   keypad(win, TRUE);
   box(win, 0, 0);
   return win;
}

void clear_and_destroy_window(WINDOW* win) {
   if(win) {
      werase(win);
      wrefresh(win);
      delwin(win);
   }
}

int calc_max_line_width(const DeckInfoList* info) {
   int max_width = 0;
   // Create formatted string for each info item and return longest string length
   for (size_t i = 0; i < info->count; ++i) {
      char buf[256];
      snprintf(buf, sizeof(buf), "%d: %s (%d cards)", info->items[i].id, info->items[i].name, info->items[i].card_count);
      int len = strlen(buf);
      if (len > max_width)
         max_width = len;
    }
    return max_width;
}


void perrorw(const char* err_msg) {
   // Print bottom left of stdscr in bold
   attron(A_BOLD);
   mvprintw(LINES - 3, 0, "%s", err_msg);
   attroff(A_BOLD);
   clrtoeol();
   refresh();
}

