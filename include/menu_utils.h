#ifndef MENU_UTILS_H
#define MENU_UTILS_H

#include <ncurses.h>
#include "db.h"

typedef enum {SHOW_FRONT, SHOW_BACK} State;

// Key Macros 
#define ESC_KEY 27
#define SPACE_KEY 32

// Return Macros 
#define RETURN_INDEX 0
#define RETURN_CUSTOM_ID 1

typedef void (*MenuItemRenderer)(WINDOW* win, int index, int highlight, void* data);

/*
* Brief - Render a generic window given generic data and renderer function
* Input - Ncurses window, number of items, title of window, generic data, renderer to use
* and return specifier
* Output - Returns line index or a custom return value specified by the renderer
*/
int generic_menu(WINDOW* win,                 // ncurses window 
                 int item_count,              // amount of items 
                 const char* title,           // menu title 
                 void* data,                  // pointer to data 
                 MenuItemRenderer renderer,   // function pointer to render data 
                 int return_id);              // return item index or ID 

/* Both renderers 
* Brief - Renders the data 
* Input - win: window to draw menu in,
*         index: index of current item or ID,
*         highlight: specifies which line to highlight,
*         data: void pointer to data to be cast into applicable datatype
* Output - None
*/
void render_string_menu_item(WINDOW* win, int index, int highlight, void* data);
void render_deck_info_item(WINDOW* win, int index, int highlight, void* data);

/*
* Brief - Renders the cards front or back 
* Input - win: window to draw card in, 
*         deck: pointer to Deck structure containing cards,
*         index: ID of current card,
*         state: flag for either the cards front or back,
*         footer: pointer for footer message for cards,
* Output - None
*/ 
void render_card(WINDOW* win, Deck* deck, int index, State state, const char* footer);

#endif

