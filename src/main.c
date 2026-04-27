#include <game.h>
#include <main_menu.h>

int main(void) {

    (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS,
    2048) != 0) ?
    printf("Error opening audio Channel\n") : (void)(0);

                // 0xb00100000; 0xb00010000
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Error initialising SDL: %s\n", SDL_GetError());
        return -1;
    }
    if((Mix_Init(MIX_INIT_OGG) & MIX_INIT_OGG) != MIX_INIT_OGG) {
        printf("Error initialising MIXER: %s\n", Mix_GetError());
    }
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        printf("Error initialising IMG: %s\n", IMG_GetError());
        return -1;
    }
    if (TTF_Init() != 0) {
        printf("Error initialising TTF: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Steichnenwolf",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    Menu* menu = menu_create(window);
    menu_init(menu);
    while(menu_is_running(menu)) {
        menu_update(menu);
    }
    menu_destroy(menu);

    Game* game = game_create(window);
    game_init(game);
    
    while(game_is_running(game)) {
        game_update(game);
    }
    
    game_destroy(game);


    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
    return 0;
}