#include <SDL2/SDL.h>
#include "../include/Game.h" 

int main(int argc, char* argv[]) {
    // Initializing SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Setting window size to 1600x900 
    Game game(1600, 900);
    game.init();
    game.run();
    game.cleanup();

    SDL_Quit();
    return 0;
}