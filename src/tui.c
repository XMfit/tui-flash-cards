#include "../include/tui.h"
#include "../include/menu_utils.h"
#include <ncurses.h>
#include <strings.h>
#include <time.h>

extern sqlite3* db;

int draw_menu(WINDOW* win, const char** choices, int n_choices, const char* title) {
    return generic_menu(win, n_choices, title, (void*)choices, render_string_menu_item, RETURN_INDEX);
}

int show_deck_info(WINDOW* parent, DeckInfoList* info) {
    int max_width = calc_max_line_width(info) + GENERIC_PADDING;
    int height = info->count + GENERIC_PADDING;

    WINDOW* win = create_centered_window(parent, height, max_width);
    int selected_id = generic_menu(win, info->count, "Deck Info", info, render_deck_info_item, RETURN_CUSTOM_ID);
    clear_and_destroy_window(win);
    return selected_id;
}

void display_cards(WINDOW* parent, Deck* deck) {
   if (deck->count == 0) {
      popup_message(parent, "This deck has no cards");
      return;
   }
   WINDOW* win = create_centered_window(parent, CARD_HEIGHT, CARD_WIDTH);
   keypad(win, TRUE);

   int index = 0;
   State state = SHOW_FRONT;
   int ch;
   
   const char* footer = "[<-] Prev  [->] Next  [SPACE] Flip [DEL] Delete [e] Edit  [ESC] Quit";
   while(1) {
      render_card(win, deck, index, state, footer);

      ch = wgetch(win);
      switch (ch) {
         case KEY_LEFT:
            if (index > 0) index--;
            state = SHOW_FRONT;
            break;
         case KEY_RIGHT:
            if (index < (int)deck->count - 1) index++;
            state = SHOW_FRONT;
            break;
         case SPACE_KEY:
            state = !state;
            break;
         case 'e':
         case 'E': {
            char edited[MAX_BUFFER] = {0};
            if (state == SHOW_FRONT) {
               form_input(stdscr, "Edit Front:", edited, MAX_BUFFER, 0);
               if(strlen(edited) > 0)
                  update_card(db, deck->items[index].id, edited, deck->items[index].back);
            } else {
               form_input(stdscr, "Edit Back:", edited, MAX_BUFFER, 0);
               if(strlen(edited) > 0)
                  update_card(db, deck->items[index].id, deck->items[index].front, edited);
            }
            load_deck_cards(db, deck->items[index].deck_id, deck);
            break;
         }
         case KEY_DC: { // deleting cards
            delete_card_by_id(db, deck->items[index].id);
            load_deck_cards(db, deck->items[index].deck_id, deck);
            if (index >= (int)deck->count)
               index = deck->count - 1;
            if (index == -1) {
               clear_and_destroy_window(win);
               return;
            }
            break;
         }
         case 10: // exit
         case ESC_KEY:
            clear_and_destroy_window(win);
            return;
         default:
            break;
      }
   }
}

void study_cards(WINDOW* parent_win, Deck* deck) {
   if (deck->count == 0) {
      popup_message(parent_win, "Deck is empty!");
      return;
   }
   srand(time(NULL));
   reset_study_flags(deck);

   WINDOW* win = create_centered_window(parent_win, CARD_HEIGHT, CARD_WIDTH);
   keypad(win, TRUE);

   State state = SHOW_FRONT;
   int ch;
   int index = rand() % (deck->count);

   while(1) {
      const char* footer = (state == SHOW_FRONT)
         ? "[SPACE] Flip Card [ESC] Quit"
         : "[Y] Correct [N] Incorrect [ESC] Quit";

      render_card(win, deck, index, state, footer);

      ch = wgetch(win);
      switch(ch) {
         case SPACE_KEY: { // Flip Card
            if (state == SHOW_FRONT)
               state = SHOW_BACK;
            break;
         }
         case 'y':
         case 'Y':
            if (state == SHOW_BACK) {
               deck->items[index].study_flag = 1; // mark as done
               // check if all cards are done
               int done = 1;
               for (size_t i = 0; i < deck->count; i++) {
                  if (!deck->items[i].study_flag) {
                     done = 0;
                     break;
                  }
               }
               if (done) {
                  popup_message(parent_win, "Study complete!");
                  clear_and_destroy_window(win);
                  return;
               }
               // pick unmarked card
               do {
                  index = rand() % deck->count;
               } while (deck->items[index].study_flag);
               state = SHOW_FRONT;
            }
            break;

         case 'n':
         case 'N':
            if (state == SHOW_BACK) {
               // keep card unmarked and pick another random card
               index = rand() % deck->count;
               state = SHOW_FRONT;
            }
            break;
         case ESC_KEY: // exit
            clear_and_destroy_window(win);
            return;
         default:
            break;
      }
   }
}

void form_input(WINDOW* parent_win, const char* form_prompt, char* input, int max_len, int dash_flag) {
   WINDOW* form_win = create_centered_window(parent_win, FORM_HEIGHT, FORM_WIDTH);
   box(form_win, 0, 0);

   wattron(form_win, A_BOLD);
   mvwprintw(form_win, 1, (FORM_WIDTH - strlen(form_prompt)) / 2, "%s", form_prompt);
   all_attr_off(form_win);
   wrefresh(form_win);
   if (!get_input_line(form_win, 2, 2, input, max_len, FORM_WIDTH - VISIBLE_WIDTH_MARGIN, dash_flag)) {
      memset(input, 0, max_len);
   }
   clear_and_destroy_window(form_win);
}

void card_input(WINDOW* parent_win, const char* form_prompt, char* input1, char* input2, int max_len) {
   WINDOW* card_win = create_centered_window(parent_win, CARD_FORM_HEIGHT, CARD_FORM_WIDTH);
   box(card_win, 0, 0);

   wattron(card_win, A_UNDERLINE);
   mvwprintw(card_win, 1, (CARD_FORM_WIDTH - strlen(form_prompt)) / 2, "%s", form_prompt);
   all_attr_off(card_win);

   wattron(card_win, A_BOLD);
   mvwprintw(card_win, 2, 2, "Front:");
   all_attr_off(card_win);
   wrefresh(card_win);

   if (!get_input_line(card_win, 3, 2, input1, max_len, CARD_FORM_WIDTH - VISIBLE_WIDTH_MARGIN, 0)) {
      memset(input1, 0, max_len);
      memset(input2, 0, max_len);
      clear_and_destroy_window(card_win);
      return;
   }
   // clear screen
   for (int i = 3; i <= CARD_FORM_HEIGHT - 2; i++)
      mvwprintw(card_win, i, 2, "%-*s", CARD_FORM_WIDTH - VISIBLE_WIDTH_MARGIN, "");

   wattron(card_win, A_BOLD);
   mvwprintw(card_win, 4, 2, "Back:");
   all_attr_off(card_win);
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

   wattron(popup_win, A_BOLD);
   mvwprintw(popup_win, 1, (POPUP_WIDTH - strlen(message)) / 2, "%s", message);
   all_attr_off(popup_win);
   mvwprintw(popup_win, POPUP_HEIGHT - 2, 2, "Press enter to close");
   wrefresh(popup_win);

   while(1) { // Close popup when enter is pressed
      ch = wgetch(popup_win);
      if (ch == '\n' || ch == KEY_ENTER)
         break;
   }
   clear_and_destroy_window(popup_win);
}
// ugly function 
int get_input_line(WINDOW* win, int y, int x, char* buffer, int max_len, int visible_width, int dash_flag) {
   int len = 0;
   int cursor = 0;
   int ch;

   const int max_lines = 4;
   keypad(win, TRUE);
   curs_set(1);
   memset(buffer, 0, max_len);

   while (1) {
      // Redraw input area
      for (int i = 0; i < max_lines; i++)
         mvwprintw(win, y + i, x, "%-*s", visible_width, "");  // Clear

      // Print wrapped lines
      for (int i = 0; i < len; i++) {
         int row = i / visible_width;
         int col = i % visible_width;
         mvwaddch(win, y + row, x + col, buffer[i]);
      }

      int cursor_row = cursor / visible_width;
      int cursor_col = cursor % visible_width;
      wmove(win, y + cursor_row, x + cursor_col);
      wrefresh(win);

      ch = wgetch(win);
      if (ch == '\n' || ch == KEY_ENTER) {
         curs_set(0);
         return 1; // Submitted
      } else if (ch == KEY_LEFT) {
            if (cursor > 0) cursor--;
      } else if (ch == KEY_RIGHT) {
            if (cursor < len) cursor++;
      } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (cursor > 0) { // memmove back one and reduce len and cursor
               memmove(&buffer[cursor - 1], &buffer[cursor], len - cursor + 1);
               cursor--;
               len--;
            }
      } else if (ch >= 32 && ch < 127) { // memmove forward one and increase len and cursor
            if (len < max_len - 1 && (len / visible_width) < max_lines) {
               memmove(&buffer[cursor + 1], &buffer[cursor], len - cursor + 1);
               buffer[cursor] = dash_flag && ch == ' ' ? '-' : ch;
               cursor++;
               len++;
            }
      } else if (ch == ESC_KEY) {
            buffer[0] = '\0';
            curs_set(0);
            return 0; // Cancelled
        }
   }
}

WINDOW* create_centered_window(WINDOW* parent, int height, int width) {
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

// Attribute funcs 
void all_attr_off(WINDOW* win) {
   wattroff(win, A_ALL_ATTRS);
}
