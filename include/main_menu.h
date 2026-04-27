#ifndef MAIN_MENU
#define MAIN_MENU

#include <game.h>

typedef struct Menu Menu;

Menu* menu_create(SDL_Window* window);
void menu_destroy(Menu* game);
SDL_bool menu_is_running(Menu* game);
void menu_init(Menu* game);
void menu_update(Menu* game);

#endif