#include "game.h"

int main(void) {
    Game* game = game_create();
    if(!game) {
        printf("Failed to create game\n");
        return 1;
    }
    
    game_init(game);
    
    while(game_is_running(game)) {
        game_update(game);
    }
    
    game_destroy(game);
    return 0;
}