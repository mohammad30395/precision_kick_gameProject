#include <SDL2/SDL.h>
#include "../include/Game.h" 
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

static Game *game = nullptr;

#ifdef __EMSCRIPTEN__
static void browserMainLoop()
{
    if (!game || !game->tick())
    {
        if (game)
        {
            game->cleanup();
            delete game;
            game = nullptr;
        }
        SDL_Quit();
        emscripten_cancel_main_loop();
    }
}
#endif

int main(int argc, char* argv[]) {
    // Initializing SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

#ifdef __EMSCRIPTEN__
    EM_ASM({
        try {
            FS.mkdir('/src');
        } catch (e) {}
        FS.chdir('/src');
    });
#endif

    // Setting window size to 1600x900 
    game = new Game(1600, 900);
    if (!game->init()) {
        delete game;
        game = nullptr;
        SDL_Quit();
        return 1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(browserMainLoop, 0, 1);
#else
    game->run();
    game->cleanup();
    delete game;
    game = nullptr;

    SDL_Quit();
#endif
    return 0;
}
