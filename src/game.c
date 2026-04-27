#include <game.h>

static const int MAP_WIDTH  = 8;
static const int MAP_HEIGHT = 8;
static const int MAP_SIZE = 8;
static const int MAP_AREA = 64;
static const int MAX_ENEMY_INSTANCES = 2;
static const int MAX_ENTITY_INSTANCES = 2;

const int map[8][8] = {
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
};


static const SDL_Color TEXT_COLOR = {0, 0, 0, 255};

typedef struct {
    float x, y, angle;
    int   health, max_health;
    int   ammo,   max_ammo;
} Player;

typedef struct {
    float x, y, health;
} Enemy;

typedef struct {
    float x, y;
} Entity;

typedef struct {
    SDL_Texture *gun;
    SDL_Texture *crosshair;
    SDL_Texture *enemy;
    TTF_Font    *font;
    Mix_Music   *gun_ogg;
} GameAssets;

typedef struct {
    SDL_Texture *health_tex;
    SDL_Texture *ammo_tex;
    int          last_health;
    int          last_ammo;
    char         hbuf[32];
    char         abuf[32];
} HUDState;

struct Game {
    SDL_bool      is_running;
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Event     event;
    Player        player;
    Enemy         enemy;
    GameAssets    assets;
    HUDState      hud;
    float         wall_distances[SCREEN_WIDTH];
    uint32_t      last_damage_time;
    uint32_t      last_event_time;
};

static Mix_Music* loadMusic(const char* fileName) {
    Mix_Music* m = NULL;
    m = Mix_LoadMUS(fileName);
    if(m == NULL) printf("Failed to load sound!\n");
    return m;
}

static int playMusic(Mix_Music* m) {
    if(Mix_PlayingMusic() == 0) {
        Mix_Volume(-1, 128);
        Mix_PlayMusic(m, 0); 
    }
}

static int playSound(Mix_Chunk* s) {
    if(Mix_PlayingMusic() == 0) {
        Mix_Volume(-1, 128);
        Mix_PlayChannel(-1, s, 0);
    }
}

static SDL_Texture *load_texture(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        printf("Error loading image '%s': %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

static SDL_Texture *render_text(TTF_Font *font, const char *text) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, TEXT_COLOR);
    if (!surface) {
        printf("Error rendering text '%s': %s\n", text, TTF_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(
        SDL_GetRenderer(SDL_GetWindowFromID(1)), surface
    );
    SDL_FreeSurface(surface);
    return tex;
}

static void update_hud_texture(SDL_Renderer *renderer, TTF_Font *font,
                                SDL_Texture **tex, char *buf, size_t buf_size,
                                const char *fmt, int value, int max_value,
                                int *last_value)
{
    if (value == *last_value) return;

    snprintf(buf, buf_size, fmt, value, max_value);
    SDL_Surface *surface = TTF_RenderText_Solid(font, buf, TEXT_COLOR);
    if (!surface) {
        printf("Error rendering HUD text: %s\n", TTF_GetError());
        return;
    }
    if (*tex) SDL_DestroyTexture(*tex);
    *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    *last_value = value;
}

static SDL_bool try_move(Game* game, const Player *p, float dx, float dy) {
    int nx = (int)(p->x + dx);
    int ny = (int)(p->y + dy);
    return map[ny][nx] != 1;
}

Game *game_create(SDL_Window* window) {
    Game *game = (Game *)calloc(1, sizeof(Game));
    game->window = window;
    if (game) game->is_running = SDL_FALSE;
    return game;
}

void game_destroy(Game *game) {
    if (!game) return;

    if (game->assets.gun)       SDL_DestroyTexture(game->assets.gun);
    if (game->assets.crosshair) SDL_DestroyTexture(game->assets.crosshair);
    if (game->assets.enemy)     SDL_DestroyTexture(game->assets.enemy);
    if (game->assets.font)      TTF_CloseFont(game->assets.font);
    if (game->hud.health_tex)   SDL_DestroyTexture(game->hud.health_tex);
    if (game->hud.ammo_tex)     SDL_DestroyTexture(game->hud.ammo_tex);
    if (game->renderer)         SDL_DestroyRenderer(game->renderer);

    free(game);
}

void game_init(Game *game) {
    if (!game) return;

    if (!game->window) {
        printf("Error creating window: %s\n", SDL_GetError());
        return;
    }

    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if (!game->renderer) {
        printf("Error creating renderer: %s\n", SDL_GetError());
        return;
    }

    SDL_RenderSetLogicalSize(game->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    game->assets.gun_ogg = loadMusic("util/gun.mp3");

    game->player = (Player){
        .x = 4.0f, .y = 4.0f, .angle = 0.0f * (180.0f/PI),
        .health = 10, .max_health = 10,
        .ammo   = 10, .max_ammo   = 10,
    };

    game->enemy = (Enemy){.x = 1.5f, .y = 1.5f, .health = 10.0f};

    game->assets.gun       = load_texture(game->renderer, "util/gun.png");
    game->assets.crosshair = load_texture(game->renderer, "util/Crosshair.png");
    game->assets.enemy     = load_texture(game->renderer, "util/enemy.png");
    game->assets.font      = TTF_OpenFont("util/arial.ttf", FONT_SIZE);

    if (!game->assets.font) {
        printf("Error loading font: %s\n", TTF_GetError());
        return;
    }

    game->hud.last_health = -1;
    game->hud.last_ammo   = -1;
    update_hud_texture(game->renderer, game->assets.font,
                       &game->hud.health_tex, game->hud.hbuf, sizeof(game->hud.hbuf),
                       "Health: %d / 100",
                       (game->player.health/game->player.max_health)*100, 0,
                       &game->hud.last_health);
    update_hud_texture(game->renderer, game->assets.font,
                       &game->hud.ammo_tex, game->hud.abuf, sizeof(game->hud.abuf),
                       "Ammo: %d / %d",
                       game->player.ammo, game->player.max_ammo,
                       &game->hud.last_ammo);

    game->last_event_time = 0;

    game->is_running = SDL_TRUE;
}


static void handle_input(Game *game) {
    Player *p = &game->player;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_W]) {
        float dx = cos(p->angle) * MOVE_SPEED;
        float dy = sin(p->angle) * MOVE_SPEED;
        if (try_move(game, p, dx, dy)) { p->x += dx; p->y += dy; }
    }
    if (keys[SDL_SCANCODE_S]) {
        float dx = -cos(p->angle) * MOVE_SPEED;
        float dy = -sin(p->angle) * MOVE_SPEED;
        if (try_move(game, p, dx, dy)) { p->x += dx; p->y += dy; }
    }
    if (keys[SDL_SCANCODE_A]) {
        float dx = cos(p->angle - PI / 2.0f) * MOVE_SPEED;
        float dy = sin(p->angle - PI / 2.0f) * MOVE_SPEED;
        if (try_move(game, p, dx, dy)) { p->x += dx; p->y += dy; }
    }
    if (keys[SDL_SCANCODE_D]) {
        float dx = cos(p->angle + PI / 2.0f) * MOVE_SPEED;
        float dy = sin(p->angle + PI / 2.0f) * MOVE_SPEED;
        if (try_move(game, p, dx, dy)) { p->x += dx; p->y += dy; }
    }

    if (keys[SDL_SCANCODE_R] && p->ammo < p->max_ammo) {
        p->ammo = p->max_ammo;
    }

    if (keys[SDL_SCANCODE_F] && p->health < p->max_health) {
        p->health = p->max_health;
        printf("Used medkit!\n");
    }
}

static void handle_events(Game *game, float enemy_angle, float enemy_distance) {
    Player *p = &game->player;
    Enemy  *e = &game->enemy;

    while (SDL_PollEvent(&game->event)) {
        SDL_Event *ev = &game->event;

        if (ev->type == SDL_QUIT) {
            game->is_running = SDL_FALSE;
        }

        if (ev->type == SDL_KEYDOWN) {
            switch (ev->key.keysym.sym) {
                case SDLK_ESCAPE:
                    game->is_running = SDL_FALSE;
                    break;
                case SDLK_e:
                    SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode());
                    break;
                default: break;
            }
        }

        if (ev->type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode()) {
            p->angle += ev->motion.xrel * ROT_SPEED;
        }

        if (ev->type == SDL_MOUSEBUTTONDOWN && p->ammo > 0) {
            uint32_t curr_time = SDL_GetTicks();
            if(curr_time - game->last_damage_time >= INPUT_INTERVAL) {
                playMusic(game->assets.gun_ogg);
                p->ammo--;
                SDL_bool on_target = fabs(enemy_angle) < SHOOT_ANGLE_TOL
                                && enemy_distance     < SHOOT_MAX_DIST
                                && enemy_distance     < game->wall_distances[SCREEN_WIDTH / 2];
                if (on_target) {
                    e->health -= 1.0f;
                    printf("Hit enemy! Health: %.0f\n", e->health);             
                    if (e->health <= 0.0f) {
                        e->x = -999.0f;
                        e->y = -999.0f;
                        printf("Enemy dead!\n");
                    }
                }
                game->last_damage_time = curr_time;
            }
        }
    }
}

static void render_scene(Game *game) {
    SDL_Renderer *r = game->renderer;
    Player *p = &game->player;

    SDL_SetRenderDrawColor(r, 100, 150, 200, 255);
    SDL_Rect ceiling = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(r, &ceiling);

    SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
    SDL_Rect floor = {0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
    SDL_RenderFillRect(r, &floor);

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        float ray_angle = (p->angle - FOV / 2.0f) + ((float)x / SCREEN_WIDTH) * FOV;
        float ray_dx = cos(ray_angle);
        float ray_dy = sin(ray_angle);

        int map_x = (int)p->x;
        int map_y = (int)p->y;

        float delta_x = fabs(1.0f / ray_dx);
        float delta_y = fabs(1.0f / ray_dy);

        float side_x, side_y;
        int step_x, step_y;

        if (ray_dx < 0) { step_x = -1; side_x = (p->x - map_x) * delta_x; }
        else            { step_x =  1; side_x = (map_x + 1.0f - p->x) * delta_x; }

        if (ray_dy < 0) { step_y = -1; side_y = (p->y - map_y) * delta_y; }
        else            { step_y =  1; side_y = (map_y + 1.0f - p->y) * delta_y; }

        int hit = 0, side = 0;
        while (!hit) {
            if (side_x < side_y) { side_x += delta_x; map_x += step_x; side = 0; }
            else                  { side_y += delta_y; map_y += step_y; side = 1; }

            if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) break;
            if (map[map_y][map_x] == 1) hit = 1;
        }

        float distance = (side == 0)
            ? (side_x - delta_x) * cos(ray_angle - p->angle)
            : (side_y - delta_y) * cos(ray_angle - p->angle);

        game->wall_distances[x] = distance;

        int wall_height = (int)(SCREEN_HEIGHT / distance);
        int draw_start  = SDL_max((SCREEN_HEIGHT - wall_height) / 2, 0);
        int draw_end    = SDL_min(draw_start + wall_height, SCREEN_HEIGHT - 1);

        int shade = SDL_max(0, 255 - (int)(distance * 50));
        if (side == 1) shade = (int)(shade * 0.7f);

        SDL_SetRenderDrawColor(r, shade, shade / 3, shade / 3, 255);
        SDL_RenderDrawLine(r, x, draw_start, x, draw_end);
    }
}

static void render_enemy(Game *game, float enemy_angle, float enemy_distance) {
    if (fabs(enemy_angle) >= FOV / 2.0f || enemy_distance <= 0.1f) return;

    int screen_x    = (int)((enemy_angle + FOV / 2.0f) / FOV * SCREEN_WIDTH);
    int enemy_height = (int)(SCREEN_HEIGHT / enemy_distance);
    int draw_start  = SDL_max((SCREEN_HEIGHT - enemy_height) / 2, 0);
    int draw_end    = SDL_min(draw_start + enemy_height, SCREEN_HEIGHT - 1);
    int enemy_width = enemy_height;

    for (int col = screen_x - enemy_width / 2; col < screen_x + enemy_width / 2; col++) {
        if (col < 0 || col >= SCREEN_WIDTH) continue;
        if (enemy_distance >= game->wall_distances[col]) continue;

        int tex_col = (col - (screen_x - enemy_width / 2)) * ENEMY_SPRITE_SIZE / enemy_width;
        SDL_Rect src = {tex_col, 0, 1, ENEMY_SPRITE_SIZE};
        SDL_Rect dst = {col, draw_start, 1, draw_end - draw_start};
        SDL_RenderCopy(game->renderer, game->assets.enemy, &src, &dst);
    }
}

static void render_hud(Game *game) {
    SDL_Renderer *r = game->renderer;
    Player *p = &game->player;

    SDL_Rect gun_rect = {SCREEN_WIDTH - 384, SCREEN_HEIGHT - 384, 384, 384};
    SDL_RenderCopy(r, game->assets.gun, NULL, &gun_rect);

    SDL_Rect xhair_rect = {SCREEN_WIDTH / 2 - 16, SCREEN_HEIGHT / 2 - 16, 32, 32};
    SDL_RenderCopy(r, game->assets.crosshair, NULL, &xhair_rect);

    update_hud_texture(r, game->assets.font,
                       &game->hud.health_tex, game->hud.hbuf, sizeof(game->hud.hbuf),
                       "Health: %d / 100", p->health, p->max_health,
                       &game->hud.last_health);
    update_hud_texture(r, game->assets.font,
                       &game->hud.ammo_tex, game->hud.abuf, sizeof(game->hud.abuf),
                       "Ammo: %d / %d", p->ammo, p->max_ammo,
                       &game->hud.last_ammo);

    SDL_Rect health_rect = {10, SCREEN_HEIGHT - 100, 100, 100};
    SDL_RenderCopy(r, game->hud.health_tex, NULL, &health_rect);

    SDL_Rect ammo_rect = {10, SCREEN_HEIGHT - 200, 100, 100};
    SDL_RenderCopy(r, game->hud.ammo_tex, NULL, &ammo_rect);
}


void game_update(Game *game) {
    if (!game || !game->is_running) return;

    Player *p = &game->player;
    Enemy  *e = &game->enemy;

    float dx = e->x - p->x;
    float dy = e->y - p->y;
    float enemy_distance = sqrtf(dx * dx + dy * dy);
    float enemy_angle    = atan2f(dy, dx) - p->angle;

    while (enemy_angle >  PI) enemy_angle -= 2.0f * PI;
    while (enemy_angle < -PI) enemy_angle += 2.0f * PI;

    handle_input(game);
    handle_events(game, enemy_angle, enemy_distance);

    if (enemy_distance < 1.5f) {
        uint32_t now = SDL_GetTicks();
        if (now - game->last_damage_time > DAMAGE_COOLDOWN_MS) {
            p->health--;
            game->last_damage_time = now;
            if (p->health <= 0) {
                printf("You died!\n");
                p->health = p->max_health;
            }
        }
    }

    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderClear(game->renderer);

    render_scene(game);
    render_enemy(game, enemy_angle, enemy_distance);
    render_hud(game);

    SDL_RenderPresent(game->renderer);
    SDL_Delay(16);
}

SDL_bool game_is_running(Game *game) {
    return game && game->is_running;
}