#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

using namespace std;

class ScreenCredits {
public:
    ScreenCredits(SDL_Renderer* renderer, TTF_Font* fontTitle, TTF_Font* fontButton, int winWidth, int winHeight);
    ~ScreenCredits();

    // Returns true if the back button is pressed this frame 
    bool handleEvent(const SDL_Event& e);

    // Draws the credits screen, including background, shadow box, text, buttons
    void render();

    bool isSoundOn() const { return soundOn; }

private:
    SDL_Renderer* renderer;
    TTF_Font* fontTitle;
    TTF_Font* fontButton;
    int windowWidth;
    int windowHeight;

    SDL_Texture* bgTexture;       // background (levelselect_bg.png)
    SDL_Texture* backBtnTexture;  // back_arrow.png
    SDL_Texture* soundOnTexture;  // music_on.png
    SDL_Texture* soundOffTexture; // music_off.png

    SDL_Rect backBtnRect;
    SDL_Rect soundBtnRect;
    SDL_Rect textBoxRect;

    bool backPressed;
    bool soundOn;

    string creditsText;

    void drawShadowBox(SDL_Rect rect, SDL_Color color, int alpha);
    SDL_Texture* createWrappedText(const string& text, SDL_Color color, int maxWidth);

    // Utility to load texture
    SDL_Texture* loadTexture(const string& path);
};