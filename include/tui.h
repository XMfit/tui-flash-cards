#ifndef TUI_H
#define TUI_H

#include <ncurses.h>
#include <string.h>
#include "db.h"

// Window Dimension Macros
#define MAIN_MENU_HEIGHT 15
#define MAIN_MENU_WIDTH 70

#define FORM_HEIGHT 5
#define FORM_WIDTH  70

#define POPUP_HEIGHT 5 
#define POPUP_WIDTH 70

#define CARD_FORM_HEIGHT 10
#define CARD_FORM_WIDTH 70

// Margin Macros
#define BOTTOM_MARGIN 5
#define VISIBLE_WIDTH_MARGIN 4

// Padding Macros 
#define GENERIC_PADDING 5

// Key Macros 
#define ESC_KEY 27
#define SPACE_KEY 32

int draw_menu(WINDOW* win, int starty, int startx, const char** choices, int n_choices, const char* title);

/*
* Brief - This function calculates the maximum amount of characters the longest formatted line is
* Input - structure containing DeckInfoList
* Output - length n
*/
int calc_max_line_width(const DeckInfoList* info);

/*
* Brief - This function displays each deck and its info to the user in a window and allows them to select 
* the deck via the keypad
* Input - parent window (usually stdscr) and DeckInfoList
* Output - ID of the selected Deck
*/
int show_deck_info(WINDOW* parent, DeckInfoList* info);

/*
* Brief - Interactive input line allowing users to type their input, and use the keypads to move the 
* cursor
* Input - Window win thats calling it, y & x cords specifying where to start input line in window,
* buffer to save line in, length of buffer, size of input box
*/
int get_input_line(WINDOW* win, int y, int x, char* buffer, int max_len, int visible_width);

/*
* Brief - Create a window centered and below the parent 
* Input - Parent window, height and width of new window 
* Output - pointer to new window
*/ 
WINDOW* create_centered_window(WINDOW* parent, int height, int width);

/*
* Brief - Clear and destroy window 
* Input - Window to destroy and clear 
* Output - 
*/
void clear_and_destroy_window(WINDOW* win);

/*
* Brief - Create input prompt from parent window 
* Input - Parent window, prompt for form, input buffer, buffer size
*/
void form_input(WINDOW* parent_win, const char* form_prompt, char* input, int max_len);

/*
* See above just include second input buffer
*/
void card_input(WINDOW* parent_win, const char* form_prompt, char* input1, char* input2, int max_len);

/*
* Brief - Create popup window displaying message 
* Input - Parent window, message to display 
*/
void popup_message(WINDOW * parent_win, const char* message);

/*
* Brief - Display error message bottom left of terminal
* Input - Error message 
*/
void perrorw(const char* err_msg);

#endif

