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

/*
* Brief - Draw a menu of choices in the given window, allowing user to navigate and select an option.
* Input - win: window to draw menu in,
*         choices: array of string choices,
*         n_choices: number of choices,
*         title: menu title string
* Output - Index of selected choice, or -1 if user cancels (ESC)
*/
int draw_menu(WINDOW* win, const char** choices, int n_choices, const char* title);

/*
* Brief - Calculate the maximum length of formatted lines describing each deck in DeckInfoList.
* Input - Pointer to DeckInfoList structure containing deck info items.
* Output - Integer representing the max line width needed for display.
*/
int calc_max_line_width(const DeckInfoList* info);

/*
* Brief - Display deck information in a window, allow user to navigate and select a deck.
* Input - parent: parent window (usually stdscr),
*         info: pointer to DeckInfoList containing decks to display
* Output - ID of the selected deck, or -1 if cancelled.
*/
int show_deck_info(WINDOW* parent, DeckInfoList* info);

/*
* Brief - Display all cards of a deck in the given window.
* Input - parent: window to draw cards in,
*         deck: pointer to Deck structure containing cards
* Output - None
*/
void display_cards(WINDOW* parent, Deck* deck);

/*
* Brief - Allow user to study the cards in the deck interactively.
* Input - parent: window to draw study interface,
*         deck: pointer to Deck structure containing cards
* Output - None
*/
void study_cards(WINDOW* parent, Deck* deck);

/*
* Brief - Interactive input line with cursor navigation and editing features.
* Input - win: window where input is received,
*         y, x: starting coordinates within the window,
*         buffer: buffer to store user input,
*         max_len: max buffer length,
*         visible_width: width of input box displayed,
*         dash_flag: flag to control behavior (e.g., allow dashes)
* Output - Number of characters entered or -1 on error.
*/
int get_input_line(WINDOW* win, int y, int x, char* buffer, int max_len, int visible_width, int dash_flag);

/*
* Brief - Create a new window centered relative to the parent window.
* Input - parent: parent window to center relative to,
*         height: height of new window,
*         width: width of new window
* Output - Pointer to the newly created centered window.
*/
WINDOW* create_centered_window(WINDOW* parent, int height, int width);

/*
* Brief - Erase contents and destroy the specified window safely.
* Input - win: window to clear and destroy
* Output - None
*/
void clear_and_destroy_window(WINDOW* win);

/*
* Brief - Display a prompt and get user input within a form interface.
* Input - parent_win: parent window for the prompt,
*         form_prompt: prompt message string,
*         input: buffer to store user input,
*         max_len: max length of input buffer,
*         dash_flag: flag to control input behavior (e.g., allow dashes)
* Output - None
*/
void form_input(WINDOW* parent_win, const char* form_prompt, char* input, int max_len, int dash_flag);

/*
* Brief - Similar to form_input but accepts two separate input buffers (e.g. for question and answer).
* Input - parent_win: parent window for the prompt,
*         form_prompt: prompt message string,
*         input1: first input buffer,
*         input2: second input buffer,
*         max_len: max length for both buffers
* Output - None
*/
void card_input(WINDOW* parent_win, const char* form_prompt, char* input1, char* input2, int max_len);

/*
* Brief - Create and display a popup window showing a message.
* Input - parent_win: parent window for centering popup,
*         message: message string to display
* Output - None
*/
void popup_message(WINDOW * parent_win, const char* message);

/*
* Brief - Display an error message at the bottom-left corner of the terminal.
* Input - err_msg: error message string
* Output - None
*/
void perrorw(const char* err_msg);

/*
* Brief - Disable all text attributes (e.g. bold, underline, reverse) on the given window.
* Input - win: window to clear attributes from
* Output - None
*/
void all_attr_off(WINDOW* win);

#endif

