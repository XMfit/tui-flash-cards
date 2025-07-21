#ifndef TUI_H
#define TUI_H

#include <ncurses.h>
#include <string.h>

#define FORM_HEIGHT 5
#define FORM_WIDTH  50

#define POPUP_HEIGHT 5
#define POPUP_WIDTH 50

#define ESC_KEY 27

int draw_menu(WINDOW* win, int starty, int startx, const char** choices, int n_choices, const char* title);
void form_input(WINDOW* parent_win, const char* form_prompt, char* input, int max_len);
void popup_message(WINDOW * parent_win, const char* message);

#endif

