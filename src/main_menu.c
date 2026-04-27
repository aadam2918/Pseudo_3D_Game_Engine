#include <main_menu.h>

static const SDL_Color TEXT_COLOR = {0, 0, 0, 255};

typedef struct {
    SDL_Texture* bg;
    SDL_Texture *logo;
    TTF_Font    *font;
} Assets;

struct Menu {
    SDL_bool is_running;
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Event e;
    Assets assets;
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


Menu* menu_create(SDL_Window* window) {
    Menu* menu = (Menu*)calloc(1, sizeof(Menu));
    menu->window = window;
    if(menu) menu->is_running = SDL_FALSE;
    return menu;
}

void menu_destroy(Menu* menu) {
    if(!menu) return;
    if(menu->renderer) SDL_DestroyRenderer(menu->renderer);

    free(menu);
}

void menu_init(Menu* menu) {
    if(!menu) return;
    
    menu->is_running = SDL_TRUE;
    
    menu->renderer = SDL_CreateRenderer(menu->window, -1, SDL_RENDERER_ACCELERATED);
    if(!menu->renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        menu->is_running = SDL_FALSE;
        return;
    }

    menu->assets.font = TTF_OpenFont("util/arial.ttf", 12);
    menu->assets.logo = load_texture(menu->renderer, "util/logo.png");
    menu->assets.bg = load_texture(menu->renderer, "util/menu.png");
}

void menu_update(Menu* menu) {
    if(!menu || !menu->is_running) return;

    
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    
    if(keys[SDL_SCANCODE_UP]) {

    }   
    
    while(SDL_PollEvent(&menu->e)) {
        if(menu->e.type == SDL_QUIT) menu->is_running = SDL_FALSE;
        if(menu->e.type == SDL_KEYDOWN && menu->e.key.keysym.sym == SDLK_SPACE) menu->is_running = SDL_FALSE;
    }

    SDL_SetRenderDrawColor(menu->renderer, 255, 255, 255, 255);
    SDL_RenderClear(menu->renderer);
    
    SDL_SetRenderDrawColor(menu->renderer, 0, 0, 0, 255);
    SDL_Rect logoRect = {.w =480, .h =270, .x = 0, .y = 0};
    SDL_RenderCopy(menu->renderer, menu->assets.logo, NULL, &logoRect);

    SDL_Rect bgRect = {.w = SCREEN_WIDTH, .h = SCREEN_HEIGHT - 20, .x = 0, .y = + 270};
    SDL_RenderCopy(menu->renderer, menu->assets.bg, NULL, &bgRect);

    SDL_RenderPresent(menu->renderer);
    SDL_Delay(16);
}

SDL_bool menu_is_running(Menu* menu) {
    return menu && menu->is_running;
}