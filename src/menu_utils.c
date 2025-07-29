#include "../include/menu_utils.h"
#include "../include/db.h"
#include "../include/tui.h"
#include <ncurses.h>

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
   "Delete Deck",
   "Back to Main Menu"
};

int generic_menu(WINDOW* win, int item_count, const char* title, void* data,
                 MenuItemRenderer renderer, int return_id) {
   int highlight = 0;
   int ch;

   keypad(win, TRUE);
   while (1) {
      werase(win);
      box(win, 0, 0);
      wattron(win, A_UNDERLINE);
      mvwprintw(win, 1, 2, "%s", title);
      all_attr_off(win);

      for (int i = 0; i < item_count; i++) {
         renderer(win, i, highlight, data);
      }

      wattron(win, A_BOLD);
      mvwprintw(win, item_count + 5, 2, "Arrow keys to navigate, Enter to select, ESC to quit");
      all_attr_off(win);
      wrefresh(win);

      ch = wgetch(win);
      switch (ch) {
         case KEY_UP:
            highlight = (highlight == 0) ? item_count - 1 : highlight - 1;
            break;
         case KEY_DOWN:
            highlight = (highlight == item_count - 1) ? 0 : highlight + 1;
            break;
         case 10: // Enter
            return return_id ? ((DeckInfoList*)data)->items[highlight].id : highlight;
         case ESC_KEY: // ESC_KEY
            return -1;
         default:
            break;
        }
    }
}

void render_string_menu_item(WINDOW* win, int index, int highlight, void* data) {
   const char** items = (const char**)data;  // cast data
   
   // print items
   if (index == highlight) wattron(win, A_REVERSE);

   mvwprintw(win, index + 3, 4, "%s", items[index]);

   if (index == highlight) wattroff(win, A_REVERSE);
}

void render_deck_info_item(WINDOW* win, int index, int highlight, void* data) {
   DeckInfoList* info = (DeckInfoList*)data;    // cast data 

   // print info
   if (index == highlight) wattron(win, A_REVERSE);

   mvwprintw(win, index + 2, 2, "%d: %s (%d cards)",
               info->items[index].id,
               info->items[index].name,
               info->items[index].card_count);

   if (index == highlight) wattroff(win, A_REVERSE);
}

void render_card(WINDOW* win, Deck* deck, int index, State state, const char* footer) {
   werase(win);
   box(win, 0, 0);

   // Card Number Display 
   wattron(win, A_UNDERLINE);
   mvwprintw(win, 1, 2, "Card %d/%zu", index + 1, deck->count);
   all_attr_off(win);

   // Show front or back 
   const char* side = (state == SHOW_FRONT) ? "Front:" : "Back:";
   const char* text = (state == SHOW_FRONT) ? deck->items[index].front : deck->items[index].back;

   wattron(win, A_BOLD);
   mvwprintw(win, 2, 2, "%s", side);
   all_attr_off(win);
   mvwprintw(win, 3, 4, "%s", text);

   // Footer 
   wattron(win, A_BOLD);
   mvwprintw(win, CARD_HEIGHT - 2, 2, "%s", footer);
   all_attr_off(win);

   wrefresh(win);
}
