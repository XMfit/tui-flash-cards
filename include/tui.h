#ifndef TUI_H
#define TUI_H

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "db.h"

// Window Dimension Macros
#define MAIN_MENU_HEIGHT 15
#define MAIN_MENU_WIDTH 70

// Window Positions
#define MAIN_MENU_Y ((LINES - MAIN_MENU_HEIGHT) / 2) - 5
#define MAIN_MENU_X ((COLS - MAIN_MENU_WIDTH) / 2)

#define FORM_HEIGHT 7
#define FORM_WIDTH  70

#define POPUP_HEIGHT 7 
#define POPUP_WIDTH 70

#define CARD_HEIGHT 10 
#define CARD_WIDTH  75

#define CARD_FORM_HEIGHT 10
#define CARD_FORM_WIDTH 70

// Margin Macros
#define BOTTOM_MARGIN 15
#define VISIBLE_WIDTH_MARGIN 4

// Padding Macros 
#define GENERIC_PADDING 5

// Key Macros 
#define ESC_KEY 27
#define SPACE_KEY 32

// Prompts 
#define CARD_PROMPT "Enter Card Information:"
#define DECKC_PROMPT "Enter deck name: "
#define DECKD_PROMPT "Enter deck to delete "

// Attributes 
#define A_ALL_ATTRS (A_NORMAL | A_STANDOUT | A_UNDERLINE | A_REVERSE | \
                     A_BLINK | A_DIM | A_BOLD | A_PROTECT | A_INVIS | \
                     A_ALTCHARSET | A_CHARTEXT)

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
* Brief - Function to display cards in a window
* Input - Parent window, Deck structure
*/
void display_cards(WINDOW* parent, Deck* deck);

/*
* Brief - Function to display cards in a window, and allow the user to study them 
* Input - Parent window, Deck structure 
*/
void study_cards(WINDOW* parent, Deck* deck);

/*
* Brief - Interactive input line allowing users to type their input, and use the keypads to move the 
* cursor
* Input - Window win thats calling it, y & x cords specifying where to start input line in window,
* buffer to save line in, length of buffer, size of input box
*/
int get_input_line(WINDOW* win, int y, int x, char* buffer, int max_len, int visible_width, int dash_flag);

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
void form_input(WINDOW* parent_win, const char* form_prompt, char* input, int max_len, int dash_flag);

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

/*
* Turn off attributes
*/ 
void all_attr_off(WINDOW* win);

#endif

