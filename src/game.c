#include <game.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define PI 3.14159265359

struct Game {
    SDL_bool is_running;
};

const float FOV = 60 * (PI/180);
float playerX;
float playerY;
float playerAngle;

const int MAP_WIDTH = 8;
const int MAP_HEIGHT = 8;

const int map[8][8] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}
};

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event e;

Game* game_create(void) {
    Game* game = (Game*)malloc(sizeof(Game));
    if(game) game->is_running = SDL_FALSE;
    return game;
}

void game_destroy(Game* game) {
    if(game) free(game);
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void game_init(Game* game) {
    if(!game) return;
    
    game->is_running = SDL_TRUE;
    
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        game->is_running = SDL_FALSE;
        return;
    }
    
    window = SDL_CreateWindow("Pseudo 3D Game", 
                              SDL_WINDOWPOS_CENTERED, 
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, 
                              SDL_WINDOW_SHOWN);
    if(!window) {
        printf("Window Error: %s\n", SDL_GetError());
        game->is_running = SDL_FALSE;
        return;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        game->is_running = SDL_FALSE;
        return;
    }

    playerX = 4.0f;
    playerY = 4.0f;
    playerAngle = 0;
}

void game_update(Game* game) {
    if(!game || !game->is_running) return;
    
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    float moveSpeed = 0.05;
    float rotSpeed = 0.03;
    
    if(keys[SDL_SCANCODE_W]) {
        if(map[(int)(playerX + cos(playerAngle) * moveSpeed)][(int)(playerY + sin(playerAngle) * moveSpeed)] != 1) {
            playerX += cos(playerAngle) * moveSpeed;
            playerY += sin(playerAngle) * moveSpeed;
        } else {
            playerX += 0.001f * cos(playerAngle) * moveSpeed;
        }
    }
    if(keys[SDL_SCANCODE_S]) {
        if(map[(int)(playerX - cos(playerAngle) * moveSpeed)][(int)(playerY - sin(playerAngle) * moveSpeed)] != 1) {
            playerX -= cos(playerAngle) * moveSpeed;
            playerY -= sin(playerAngle) * moveSpeed;
        }
    }
    if(keys[SDL_SCANCODE_A]) {
        playerAngle -= rotSpeed;
    }
    if(keys[SDL_SCANCODE_D]) {
        playerAngle += rotSpeed;
    }
    
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) game->is_running = SDL_FALSE;
        if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) game->is_running = SDL_FALSE;
    }
    
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
    SDL_Rect ceiling = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &ceiling);
    
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect floor = {0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &floor);
    
    for(int x = 0; x < SCREEN_WIDTH; x++) {
        float rayAngle = (playerAngle - FOV/2) + ((float)x / SCREEN_WIDTH) * FOV;
        
        float rayDirX = cos(rayAngle);
        float rayDirY = sin(rayAngle);
        
        int mapX = (int)playerX;
        int mapY = (int)playerY;
        
        float deltaDistX = fabs(1.0f / rayDirX);
        float deltaDistY = fabs(1.0f / rayDirY);
        
        float sideDistX, sideDistY;
        int stepX, stepY;
        
        if(rayDirX < 0) {
            stepX = -1;
            sideDistX = (playerX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - playerX) * deltaDistX;
        }
        
        if(rayDirY < 0) {
            stepY = -1;
            sideDistY = (playerY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - playerY) * deltaDistY;
        }
        
        int hit = 0;
        int side = 0;
        
        while(hit == 0) {
            if(sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if(mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
                hit = 1;
                break;
            }
            
            if(map[mapY][mapX] == 1) hit = 1;
        }
        
        float distance;
        if(side == 0) distance = (sideDistX - deltaDistX);
        else distance = (sideDistY - deltaDistY);
        
        distance = distance * cos(rayAngle - playerAngle);
        
        int wallHeight = (int)(SCREEN_HEIGHT / distance);
        int drawStart = (SCREEN_HEIGHT - wallHeight) / 2;
        if(drawStart < 0) drawStart = 0;
        int drawEnd = drawStart + wallHeight;
        if(drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;
        
        int shade = 255 - (int)(distance * 50);
        if(shade < 0) shade = 0;
        if(side == 1) shade = shade * 0.7;
        
        SDL_SetRenderDrawColor(renderer, shade, shade, shade, 255);
        SDL_RenderDrawLine(renderer, x, drawStart, x, drawEnd);
    }
    
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
}

SDL_bool game_is_running(Game* game) {
    return game && game->is_running;
}