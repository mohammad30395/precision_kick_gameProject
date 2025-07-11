#include "../include/ScreenLevelSelect.h"
#include <SDL2/SDL_image.h>
#include <iostream>

using namespace std;

// load textures safely
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

ScreenLevelSelect::ScreenLevelSelect(SDL_Renderer* renderer, TTF_Font* fontTitle, TTF_Font* fontButton, int winWidth, int winHeight)
    : renderer(renderer), fontTitle(fontTitle), fontButton(fontButton),
      bg(nullptr), arrow(nullptr), windowWidth(winWidth), windowHeight(winHeight)
{
    // Load background and arrow textures from assets
    bg = loadTexture(renderer, "../assets/levelselect_bg.png");
    arrow = loadTexture(renderer, "../assets/back_arrow.png");

    // Centered button layout 
    int buttonWidth = 400;
    int buttonHeight = 90;
    int gapY = 55;
    int totalHeight = 3 * buttonHeight + 2 * gapY;
    int startY = windowHeight / 2 - totalHeight / 2;
    int centerX = windowWidth / 2 - buttonWidth / 2;

    buttons.clear();
    buttons.emplace_back(centerX, startY + 0 * (buttonHeight + gapY), buttonWidth, buttonHeight, "LEVEL 1");
    buttons.emplace_back(centerX, startY + 1 * (buttonHeight + gapY), buttonWidth, buttonHeight, "LEVEL 2");
    buttons.emplace_back(centerX, startY + 2 * (buttonHeight + gapY), buttonWidth, buttonHeight, "LEVEL 3");

    // Back arrow
    arrowRect = {32, 32, 100, 100};

    // By default only 1evel 1 unlocked
    levelUnlocked = {true, false, false};
}

ScreenLevelSelect::~ScreenLevelSelect() {
    if(bg) SDL_DestroyTexture(bg);
    if(arrow) SDL_DestroyTexture(arrow);
    arrow = nullptr;
}

// level lock / unlock
void ScreenLevelSelect::setLevelUnlocked(const vector<bool>& unlocked) {
    levelUnlocked = unlocked;
    if (levelUnlocked.size() < buttons.size())
        levelUnlocked.resize(buttons.size(), false);
    else if (levelUnlocked.size() > buttons.size())
        levelUnlocked.resize(buttons.size());
}

void ScreenLevelSelect::render() {
    // Draw background
    if(bg) SDL_RenderCopy(renderer, bg, NULL, NULL);
    else { SDL_SetRenderDrawColor(renderer,60,150,60,255); SDL_RenderClear(renderer); }

    // Draw title 
    SDL_Color gold = {255, 211, 59, 255};
    if (fontTitle) {
        SDL_Surface* ts = TTF_RenderUTF8_Blended(fontTitle, "SELECT LEVEL", gold);
        SDL_Texture* tt = SDL_CreateTextureFromSurface(renderer, ts);
        SDL_Rect tr = {windowWidth/2 - ts->w/2, 110, ts->w, ts->h};
        SDL_RenderCopy(renderer, tt, NULL, &tr);
        SDL_FreeSurface(ts); SDL_DestroyTexture(tt);
    }

    // Draw level buttons (dim if locked)
    for (size_t i = 0; i < buttons.size(); ++i) {
        SDL_Color btncol = {109,80,48,255};
        SDL_Color txtcol = {255,211,59,255};
        if (i >= levelUnlocked.size() || !levelUnlocked[i]) {
            btncol = {80, 80, 80, 180};     // Dim color
            txtcol = {180, 180, 180, 180};  // Gray out text
        }
        buttons[i].render(renderer, fontButton, btncol, txtcol);

        // Draw a lock icon or overlay if locked
        if (i >= levelUnlocked.size() || !levelUnlocked[i]) {
            // Drawing a lock icon or text
            TTF_Font* lockFont = TTF_OpenFont("../assets/font.ttf", 24);
            if (lockFont) {
                SDL_Color lockColor = {200, 40, 40, 255};
                SDL_Surface* surf = TTF_RenderText_Solid(lockFont, "LOCKED", lockColor);
                if (surf) {
                    SDL_Texture* txt = SDL_CreateTextureFromSurface(renderer, surf);
                    int tw = 0, th = 0;
                    SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                    SDL_Rect dst = {
                        buttons[i].rect.x + buttons[i].rect.w/2 - tw/2,
                        buttons[i].rect.y + buttons[i].rect.h/2 - th/2,
                        tw, th
                    };
                    SDL_RenderCopy(renderer, txt, NULL, &dst);
                    SDL_FreeSurface(surf);
                    SDL_DestroyTexture(txt);
                }
                TTF_CloseFont(lockFont);
            }
        }
    }

    // Draw back arrow
    if(arrow) SDL_RenderCopy(renderer, arrow, NULL, &arrowRect);
    else {
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderFillRect(renderer, &arrowRect);
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        for (int i = 0; i < 60; ++i)
            SDL_RenderDrawLine(renderer, arrowRect.x+60, arrowRect.y+20+i, arrowRect.x+25, arrowRect.y+40);
    }
}

// Handles events for buttons
int ScreenLevelSelect::handleEvent(SDL_Event& e) {
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    for (size_t i = 0; i < buttons.size(); ++i) {
        buttons[i].setHovered(mx, my);

        // Prevent clicking locked levels
        if (i >= levelUnlocked.size() || !levelUnlocked[i])
            continue;

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            if (buttons[i].isClicked(mx, my)) return i;
        }
    }
    return -1;
}

// Checks if back arrow is clicked
bool ScreenLevelSelect::backClicked(SDL_Event& e) {
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        return (mx >= arrowRect.x && mx <= arrowRect.x+arrowRect.w &&
                my >= arrowRect.y && my <= arrowRect.y+arrowRect.h);
    return false;
}