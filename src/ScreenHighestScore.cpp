#include "../include/ScreenHighestScore.h"
#include <SDL2/SDL_image.h>
#include <iostream>

using namespace std;

// utility to load texture
static SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& path) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        cout << "IMG_Load Error: " << IMG_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

// constructor
ScreenHighestScore::ScreenHighestScore(SDL_Renderer* renderer, TTF_Font* titleFont, TTF_Font* buttonFont, int winW, int winH)
    : renderer(renderer), titleFont(titleFont), buttonFont(buttonFont),
      windowWidth(winW), windowHeight(winH), backHovered(false)
{
    backgroundTexture = loadTexture(renderer, "../assets/levelselect_bg.png");
    backArrowTexture  = loadTexture(renderer, "../assets/back_arrow.png");
    backArrowRect = {32, 32, 56, 56};
}

ScreenHighestScore::~ScreenHighestScore()
{
    //---DEBUG CODE--- cout << "ScreenHighestScore destroyed: " << this << endl;
    if (backgroundTexture) {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
    if (backArrowTexture) {
        SDL_DestroyTexture(backArrowTexture);
        backArrowTexture = nullptr;
    }
}

// highest score screen
void ScreenHighestScore::render(const string& playerName, const vector<int>& scores, int totalScore)
{
    // Background
    if (backgroundTexture)
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    // Dim overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120); 
    SDL_Rect dimRect = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &dimRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // HIGHEST SCORE Title
    TTF_Font* font = titleFont;
    SDL_Color color = {255, 215, 0, 255};
    SDL_Surface* surf = TTF_RenderText_Solid(font, "HIGHEST SCORE", color);
    if (surf) {
        SDL_Texture* txt = SDL_CreateTextureFromSurface(renderer, surf);
        int tw, th; SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
        SDL_Rect dst = {windowWidth/2 - tw/2, 60, tw, th};
        SDL_RenderCopy(renderer, txt, NULL, &dst);
        SDL_FreeSurface(surf); SDL_DestroyTexture(txt);
    }

    // Player name
    font = buttonFont;
    SDL_Color nameColor = {80, 200, 255, 255};
    surf = TTF_RenderText_Solid(font, playerName.c_str(), nameColor);
    if (surf) {
        SDL_Texture* txt = SDL_CreateTextureFromSurface(renderer, surf);
        int tw, th; SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
        SDL_Rect dst = {windowWidth/2 - tw/2, 140, tw, th};
        SDL_RenderCopy(renderer, txt, NULL, &dst);
        SDL_FreeSurface(surf); SDL_DestroyTexture(txt);
    }

    // Per-level scores and total
    font = buttonFont;
    SDL_Color scoreColor = {255, 255, 255, 255};
    int baseY = 220, lineH = 50;
    for (int i = 0; i < (int)scores.size(); ++i) {
        string label = "Level " + to_string(i+1) + ": " + to_string(scores[i]);
        surf = TTF_RenderText_Solid(font, label.c_str(), scoreColor);
        if (surf) {
            SDL_Texture* txt = SDL_CreateTextureFromSurface(renderer, surf);
            int tw, th; SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
            SDL_Rect dst = {windowWidth/2 - tw/2, baseY + i*lineH, tw, th};
            SDL_RenderCopy(renderer, txt, NULL, &dst);
            SDL_FreeSurface(surf); SDL_DestroyTexture(txt);
        }
    }
    string totalLabel = "TOTAL: " + to_string(totalScore);
    surf = TTF_RenderText_Solid(font, totalLabel.c_str(), scoreColor);
    if (surf) {
        SDL_Texture* txt = SDL_CreateTextureFromSurface(renderer, surf);
        int tw, th; SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
        SDL_Rect dst = {windowWidth/2 - tw/2, baseY + (int)scores.size()*lineH + 24, tw, th};
        SDL_RenderCopy(renderer, txt, NULL, &dst);
        SDL_FreeSurface(surf); SDL_DestroyTexture(txt);
    }

    // Drawing back arrow
    if (backArrowTexture)
        SDL_RenderCopy(renderer, backArrowTexture, NULL, &backArrowRect);

    // highlight arrow if hovered
    if (backHovered) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 80);
        SDL_RenderFillRect(renderer, &backArrowRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

bool ScreenHighestScore::handleEvent(const SDL_Event& e)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_Point pt = {mx, my};
    backHovered = SDL_PointInRect(&pt, &backArrowRect);

    // if mouse click on back arrow
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && backHovered)
        return true;
    return false;
}