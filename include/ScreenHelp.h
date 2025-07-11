#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

using namespace std;

class ScreenHelp {
public:
    ScreenHelp(SDL_Renderer* renderer, TTF_Font* fontTitle, TTF_Font* fontButton, int winWidth, int winHeight);
    ~ScreenHelp();

    // Returns true if the BACK button is pressed this frame 
    bool handleEvent(const SDL_Event& e);

    // Draws the help screen, including background, shadow box, text, buttons
    void render();

    // optionally connect sound toggle to global state
    bool isSoundOn() const { return soundOn; }

private:
    SDL_Renderer* renderer;
    TTF_Font* fontTitle;
    TTF_Font* fontButton;
    int windowWidth;
    int windowHeight;

    SDL_Texture* bgTexture;       // background 
    SDL_Texture* backBtnTexture;  // back.png
    SDL_Texture* soundOnTexture;  // music_on.png
    SDL_Texture* soundOffTexture; // music_off.png

    SDL_Rect backBtnRect;
    SDL_Rect soundBtnRect;
    SDL_Rect textBoxRect;

    bool backPressed;
    bool soundOn;

    string helpText;

    // Internal helpers
    void drawShadowBox(SDL_Rect rect, SDL_Color color, int alpha);
    SDL_Texture* createWrappedText(const std::string& text, SDL_Color color, int maxWidth);

    // Utility to load texture
    SDL_Texture* loadTexture(const std::string& path);
};