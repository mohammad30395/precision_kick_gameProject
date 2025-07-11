#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include "Button.h"

using namespace std;

// Represents the Home Screen UI
class ScreenHome {
public:
    // Constructors
    ScreenHome(SDL_Renderer* renderer, TTF_Font* fontTitle, TTF_Font* fontButton, int winWidth, int winHeight);
    ~ScreenHome();

    // Renders the Home screen, optionally with username and continue button
    void render(const string& userName = "", bool canContinue = false);

    // Handles events
    int handleEvent(SDL_Event& e, bool canContinue = false);

    void setShowContinue(bool show);

    bool getShowContinue() const { return showContinue; }

private:
    SDL_Renderer* renderer;
    TTF_Font* fontTitle;
    TTF_Font* fontButton;
    SDL_Texture* bg;
    SDL_Texture* messi;
    vector<Button> buttons;

    int windowWidth;
    int windowHeight;

    // CONTINUE button layout 
    Button continueButton;
    bool continueHovered;

    bool showContinue = false; // Track if CONTINUE should be shown
    void updateButtons();      // Helper to rebuild button list
};