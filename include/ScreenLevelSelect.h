#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include "Button.h"

using namespace std;

// Represents the Level Selection screen UI
class ScreenLevelSelect {
public:
    // Constructors 
    ScreenLevelSelect(SDL_Renderer* renderer, TTF_Font* fontTitle, TTF_Font* fontButton, int winWidth, int winHeight);

    // Destructor cleans up textures
    ~ScreenLevelSelect();

    // Renders the Level Select screen
    void render();

    // Handles mouse/keyboard events. Returns int for which level button was pressed
    int handleEvent(SDL_Event& e);

    // Returns true if the back arrow was clicked
    bool backClicked(SDL_Event& e);

    // Sets which levels are unlocked for the current player
    void setLevelUnlocked(const vector<bool>& unlocked);

private:
    SDL_Renderer* renderer;         // SDL renderer for drawing
    TTF_Font* fontTitle;            // Font for screen title
    TTF_Font* fontButton;           // Font for level buttons
    SDL_Texture* bg;                // Level select background texture
    SDL_Texture* arrow;             // Back arrow texture
    vector<Button> buttons;    // Level select buttons
    SDL_Rect arrowRect;             // Rectangle for the back arrow (position/size)
    int windowWidth;                
    int windowHeight;              
    vector<bool> levelUnlocked; // Unlocked flags for levels 
};