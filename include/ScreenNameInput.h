#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

using namespace std;

class ScreenNameInput {
public:
    ScreenNameInput(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontButton, int winWidth, int winHeight);
    ~ScreenNameInput();

    void render();
    // Returns: 1 for continue, -1 for back, 0 for nothing yet
    int handleEvent(const SDL_Event& e, string& playerName);

    string getName() const { return inputText; }

    void setName(const string& name);

    void reset();

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* fontButton;
    int windowWidth, windowHeight;
    string inputText;
    bool done;
    bool active;
    SDL_Rect inputRect;
    SDL_Rect continueRect;
    bool continueHovered;

    // for back arrow support
    SDL_Texture* backArrow;
    SDL_Rect backRect;
    bool backHovered;

    // Background
    SDL_Texture* bgTexture;

    // Cursor blink state
    Uint32 lastCursorToggle;
    bool cursorVisible;

};