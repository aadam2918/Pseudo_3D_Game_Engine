#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define WINDOW_SCALE  1.5
#define WINDOW_WIDTH  (int)(SCREEN_WIDTH  * WINDOW_SCALE)
#define WINDOW_HEIGHT (int)(SCREEN_HEIGHT * WINDOW_SCALE)

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Surface* screen;
extern SDL_Surface* gun_surface;
extern SDL_Surface* crosshair_s;
extern SDL_Texture* crosshair;
extern SDL_Texture* gun;
extern SDL_Surface* health;
extern SDL_Surface* ammo;
extern SDL_Texture* h_t;
extern SDL_Texture* a_t;
extern TTF_Font* arial;
extern SDL_Surface* enemy_surface;
extern SDL_Texture* enemy_t;
extern SDL_Event e;

typedef struct Game Game;

Game* game_create(void);
void game_destroy(Game* game);
SDL_bool game_is_running(Game* game);
void game_init(Game* game);
void game_update(Game* game);

#endif