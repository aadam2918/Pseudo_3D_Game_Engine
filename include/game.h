#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Event e;
extern SDL_Rect player;

typedef struct Game Game;

Game* game_create(void);
void game_destroy(Game* game);
SDL_bool game_is_running(Game* game);
void game_init(Game* game);
void game_update(Game* game);

#endif