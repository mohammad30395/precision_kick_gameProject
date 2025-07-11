#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

using namespace std;

// Class to show the Highest Score screen
class ScreenHighestScore
{
public:
    // constructors
    ScreenHighestScore(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* buttonFont, int winW, int winH);
    ~ScreenHighestScore();

    // Drawing leaderboard for a specific player
    void render(const string& playerName, const vector<int>& scores, int totalScore);

    // Returns true if back arrow was clicked
    bool handleEvent(const SDL_Event& e);

private:
    SDL_Renderer* renderer;
    TTF_Font* titleFont;
    TTF_Font* buttonFont;
    int windowWidth, windowHeight;

    // Graphics for this screen
    SDL_Texture* backgroundTexture;
    SDL_Texture* backArrowTexture;

    SDL_Rect backArrowRect;
    bool backHovered;
};