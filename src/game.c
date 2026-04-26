#include <game.h>

#define PI 3.14159265359

struct Game {
    SDL_bool is_running;
};

const float FOV = (PI/3);
float playerX;
float playerY;
float playerAngle;

float enemyX;
float enemyY;
float enemyHealth;

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
SDL_Texture* gun = NULL;
SDL_Texture* crosshair = NULL;
SDL_Surface* gun_surface = NULL;
SDL_Surface* screen = NULL;
SDL_Surface* crosshair_s = NULL;
SDL_Surface* health = NULL;
SDL_Surface* ammo = NULL;
SDL_Texture* h_t = NULL;
SDL_Texture* a_t = NULL;
SDL_Surface* enemy_surface = NULL;
SDL_Texture* enemy_t = NULL;
TTF_Font* arial = NULL;

float wallDistances[SCREEN_WIDTH];
int pHealth, pMaxHealth, pAmmo, pMaxAmmo;

Uint32 lastDamageTime = 0;

Game* game_create(void) {
    Game* game = (Game*)malloc(sizeof(Game));
    if(game) game->is_running = SDL_FALSE;
    return game;
}

void game_destroy(Game* game) {
    if(game) free(game);
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);
    if(gun_surface) SDL_FreeSurface(gun_surface);
    if(arial) TTF_Quit();
    if(gun) SDL_DestroyTexture(gun);
    SDL_Quit();
}

void game_init(Game* game) {
    if(!game) return;
    
    game->is_running = SDL_TRUE;
    
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error initialising SDL: %s\n", SDL_GetError());
        game->is_running = SDL_FALSE;
        return;
    }

    if(IMG_Init(IMG_INIT_PNG) < 0) {
        printf("Error initalising IMG: %s\n", IMG_GetError());
        game->is_running = SDL_FALSE;
        return;
    }

    if(TTF_Init() != 0) {
        printf("Error initialising TTF %s\n", TTF_GetError());
        game->is_running = SDL_FALSE;
        return;
    }
    
    window = SDL_CreateWindow("Raja Royale 2027", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(!window) {
        printf("Error creating window: %s\n", SDL_GetError());
        game->is_running = SDL_FALSE;
        return;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        printf("Error creating renderer: %s\n", SDL_GetError());
        game->is_running = SDL_FALSE;
        return;
    }

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    playerX = 4.0f;
    playerY = 4.0f;
    playerAngle = 0;

    enemyX = 1.5f;
    enemyY = 1.5f;
    enemyHealth = 10.0f;

    pMaxHealth = 10;
    pHealth = 10;
    
    pMaxAmmo = 10;
    pAmmo = 10;

    enemy_surface = IMG_Load("util/enemy.png");
    if(!enemy_surface) {
        printf("Error loading enemy sprite: %s\n", IMG_GetError());
    }
    enemy_t = SDL_CreateTextureFromSurface(renderer, enemy_surface);
    SDL_FreeSurface(enemy_surface);
    enemy_surface = NULL;

    gun_surface = IMG_Load("util/gun.png");
    if(!gun_surface) {
        printf("Error loading gun: %s\n", IMG_GetError());
    }
    gun = SDL_CreateTextureFromSurface(renderer, gun_surface);
    SDL_FreeSurface(gun_surface);
    gun_surface = NULL;

    crosshair_s = IMG_Load("util/Crosshair.png");
    if(!crosshair_s) {
        printf("Error loading crosshair: %s\n", IMG_GetError());
    }
    crosshair = SDL_CreateTextureFromSurface(renderer, crosshair_s);
    SDL_FreeSurface(crosshair_s);
    crosshair_s = NULL;

    arial = TTF_OpenFont("util/arial.ttf", 100);

    if(!arial) {
        printf("Error loading Font\n");
        return;
    }   
}

void game_update(Game* game) {
    if(!game || !game->is_running) return;

    SDL_Color tc = {.r = 0, .g = 0, .b = 0, .a = 255};

    char hbuf[32], abuf[32];
    snprintf(hbuf, sizeof(hbuf), "Health: %d / %d", pHealth, pMaxHealth);
    snprintf(abuf, sizeof(abuf), "Ammo: %d / %d", pAmmo, pMaxAmmo);

    SDL_Surface* hs = TTF_RenderText_Solid(arial, hbuf, tc);
    fflush(stdout);
    if(!hs) {
        printf("Error loading health surface: %s\n", TTF_GetError());
    } else {
        if(h_t) SDL_DestroyTexture(h_t);
        h_t = SDL_CreateTextureFromSurface(renderer, hs);
        SDL_FreeSurface(hs);
    }
    SDL_Surface* as = TTF_RenderText_Solid(arial, abuf, tc);
    fflush(stdout);
    if(!as) {
        printf("Error loading ammo surface: %s\n", TTF_GetError());
    } else {
        if(a_t) SDL_DestroyTexture(a_t);
        a_t = SDL_CreateTextureFromSurface(renderer, as);
        SDL_FreeSurface(as);
    }
    
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    float moveSpeed = 0.05;
    float rotSpeed = 0.03;
    
    if(keys[SDL_SCANCODE_W]) {
        if(map[(int)(playerY + sin(playerAngle) * moveSpeed)][(int)(playerX + cos(playerAngle) * moveSpeed)] != 1 && playerX + cos(playerAngle) * moveSpeed != enemyX && playerY + sin(playerAngle) * moveSpeed != enemyY) {
            playerX += cos(playerAngle) * moveSpeed;
            playerY += sin(playerAngle) * moveSpeed;
        }
    }
    if(keys[SDL_SCANCODE_S]) {
        if(map[(int)(playerY - sin(playerAngle) * moveSpeed)][(int)(playerX - cos(playerAngle) * moveSpeed)] != 1 && playerX - cos(playerAngle) * moveSpeed != enemyX && playerY - sin(playerAngle) * moveSpeed != enemyY) {
            playerX -= cos(playerAngle) * moveSpeed;
            playerY -= sin(playerAngle) * moveSpeed;
        }
    }
    
    if(keys[SDL_SCANCODE_A]) {
    if(map[(int)(playerY + sin(playerAngle - PI/2) * moveSpeed)][(int)(playerX + cos(playerAngle - PI/2) * moveSpeed)] != 1) {
        playerX += cos(playerAngle - PI/2) * moveSpeed;
        playerY += sin(playerAngle - PI/2) * moveSpeed;
    }
    }
    if(keys[SDL_SCANCODE_D]) {
        if(map[(int)(playerY + sin(playerAngle + PI/2) * moveSpeed)][(int)(playerX + cos(playerAngle + PI/2) * moveSpeed)] != 1) {
            playerX += cos(playerAngle + PI/2) * moveSpeed;
            playerY += sin(playerAngle + PI/2) * moveSpeed;
        }
    }

    if(keys[SDL_SCANCODE_R] && pAmmo < pMaxAmmo) {
        printf("Reloading gun.\n");
        SDL_Delay(150);
        printf("Reloading gun..\n");
        SDL_Delay(150);
        printf("Reloading gun...\n");
        pAmmo = pMaxAmmo;
        printf("Reloaded Gun!\n");
    }

    if(keys[SDL_SCANCODE_F] && pHealth < pMaxHealth) {
        pHealth = pMaxHealth;
        printf("Used medkit!\n");
    }

    float dx = enemyX - playerX;
    float dy = enemyY - playerY;

    float enemy_distance = sqrt(dx*dx + dy*dy);
    float enemyAngle = atan2(dy, dx) - playerAngle;

    while(enemyAngle > PI) enemyAngle -= 2*PI;
    while(enemyAngle < -PI) enemyAngle += 2*PI;
        
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) game->is_running = SDL_FALSE;
        if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) game->is_running = SDL_FALSE;
        if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_e && SDL_GetRelativeMouseMode() == SDL_FALSE) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        } else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_e && SDL_GetRelativeMouseMode() == SDL_TRUE)
            SDL_SetRelativeMouseMode(SDL_FALSE);
        if(e.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode()) {
            playerAngle += e.motion.xrel * 0.001f;
        }
        if(e.type == SDL_MOUSEBUTTONDOWN && pAmmo > 0) {
            pAmmo--;
            if(fabs(enemyAngle) < 0.1f && enemy_distance < 10.0f && enemy_distance < wallDistances[SCREEN_WIDTH/2]) {
                enemyHealth -= 1.0f;
                printf("Hit Enemy! Enemy health: %.0f\n", enemyHealth);
                if(enemyHealth <= 0) {
                    enemyX = -999.0f;
                    enemyY = -999.0f;
                    printf("Enemy dead!\n");
                }
            }
        }
    }
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, 100, 150, 200, 255);
    SDL_Rect ceiling = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &ceiling);

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect floor = {0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &floor);
    
    for(int x = 0; x < SCREEN_WIDTH; x++) {
        float ray_angle = (playerAngle - FOV/2) + ((float)x / SCREEN_WIDTH) * FOV;
        
        float ray_dir_x= cos(ray_angle);
        float ray_dir_y = sin(ray_angle);
        
        int mapX = (int)playerX;
        int mapY = (int)playerY;
        
        float delta_dist_x = fabs(1.0f / ray_dir_x);
        float delta_dist_y = fabs(1.0f / ray_dir_y);
        
        float side_dist_x, side_dist_y;
        int step_x, step_y;
        
        if(ray_dir_x < 0) {
            step_x = -1;
            side_dist_x = (playerX - mapX) * delta_dist_x;
        } else {
            step_x = 1;
            side_dist_x = (mapX + 1.0f - playerX) * delta_dist_x;
        }
        
        if(ray_dir_y < 0) {
            step_y = -1;
            side_dist_y = (playerY - mapY) * delta_dist_y;
        } else {
            step_y = 1;
            side_dist_y = (mapY + 1.0f - playerY) * delta_dist_y;
        }
        
        int hit = 0;
        int side = 0;
        
        while(hit == 0) {
            if(side_dist_x < side_dist_y) {
                side_dist_x += delta_dist_x;
                mapX += step_x;
                side = 0;
            } else {
                side_dist_y += delta_dist_y;
                mapY += step_y;
                side = 1;
            }
            if(mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
                hit = 1;
                break;
            }
            
            if(map[mapY][mapX] == 1) hit = 1;
        }
        
        float distance;
        if(side == 0) distance = (side_dist_x - delta_dist_x);
        else distance = (side_dist_y - delta_dist_y);
        
        distance = distance * cos(ray_angle - playerAngle);
        wallDistances[x] = distance;
        
        int wallHeight = (int)(SCREEN_HEIGHT / distance);
        int drawStart = (SCREEN_HEIGHT - wallHeight) / 2;
        if(drawStart < 0) drawStart = 0;
        int drawEnd = drawStart + wallHeight;
        if(drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;
        
        int shade = 255 - (int)(distance * 50);
        if(shade < 0) shade = 0;
        if(side == 1) shade = shade * 0.7;
        
        SDL_SetRenderDrawColor(renderer, shade, shade / 3, shade / 3, 255);
        SDL_RenderDrawLine(renderer, x, drawStart, x, drawEnd);
    }


    if(fabs(enemyAngle) < FOV / 2 && enemy_distance > 0.1f) {
        int enemyScreenX = (int)((enemyAngle + FOV/2) / FOV * SCREEN_WIDTH);
        int enemyHeight = (int)(SCREEN_HEIGHT / enemy_distance);
        int drawStart = (SCREEN_HEIGHT - enemyHeight) / 2;
        if(drawStart < 0) drawStart = 0;
        int drawEnd = drawStart + enemyHeight;
        if(drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;
        int enemyWidth = enemyHeight;

        for(int col = enemyScreenX - enemyWidth/2; col < enemyScreenX + enemyWidth/2; col++) {
            if(col < 0 || col >= SCREEN_WIDTH) continue;
            if(enemy_distance >= wallDistances[col]) continue;

            int texCol = (col - (enemyScreenX - enemyWidth/2)) * 32 / enemyWidth;

            SDL_Rect srcRect = {texCol, 0, 1, 32};
            SDL_Rect dstRect = {col, drawStart, 1, drawEnd - drawStart};
            SDL_RenderCopy(renderer, enemy_t, &srcRect, &dstRect);
        }

    }

    if(enemy_distance < 1.5f) {
        Uint32 now = SDL_GetTicks();
        if(now - lastDamageTime > 1000) {
            pHealth -= 1;
            lastDamageTime = now;
            if(pHealth <= 0) {
                printf("You died\n");
                pHealth = pMaxHealth;
            }
    }
}

    SDL_Rect gunRect = {SCREEN_WIDTH - 384, SCREEN_HEIGHT - 384, 384, 384};
    SDL_RenderCopy(renderer, gun, NULL, &gunRect);

    SDL_Rect crosshairRect = {SCREEN_WIDTH / 2 - 16, SCREEN_HEIGHT / 2 - 16, 32, 32};
    SDL_RenderCopy(renderer, crosshair, NULL, &crosshairRect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect healthRect = {10, SCREEN_HEIGHT - 100, 100, 100};
    SDL_RenderCopy(renderer, h_t, NULL, &healthRect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect ammoRect = {10, SCREEN_HEIGHT - 200, 100, 100};
    SDL_RenderCopy(renderer, a_t, NULL, &ammoRect);    

    SDL_RenderPresent(renderer);    
    SDL_Delay(16);
}

SDL_bool game_is_running(Game* game) {
    return game && game->is_running;
}