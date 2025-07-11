#include "../include/ScreenHome.h"
#include <SDL2/SDL_image.h>
#include <iostream>

using namespace std;

// loading texture
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

ScreenHome::ScreenHome(SDL_Renderer* renderer, TTF_Font* fontTitle, TTF_Font* fontButton, int winWidth, int winHeight)
    : renderer(renderer), fontTitle(fontTitle), fontButton(fontButton),
      bg(nullptr), messi(nullptr), windowWidth(winWidth), windowHeight(winHeight),
      continueButton(winWidth * 0.61, 190, 420, 72, "CONTINUE"), continueHovered(false)
{
    bg = loadTexture(renderer, "../assets/homescreen_bg.png");
    //messi = loadTexture(renderer, "../assets/messi_character.png");

    updateButtons();
}

void ScreenHome::setShowContinue(bool show) {
    if (showContinue != show) {
        showContinue = show;
        updateButtons();
    }
}

//HOME Screen Buttons
void ScreenHome::updateButtons() {
    buttons.clear();
    int buttonWidth = 420;
    int buttonHeight = 72;
    int gapY = 32;
    int startY = 300;
    int rightX = static_cast<int>(windowWidth * 0.61);

    vector<string> labels;
    if (showContinue)
        labels.push_back("CONTINUE");

    // MENU ORDER:
    labels.push_back("NEW GAME");        
    labels.push_back("HIGHEST SCORE");   
    labels.push_back("HELP");            
    labels.push_back("CREDITS");         
    labels.push_back("EXIT");            

    for (int i = 0; i < (int)labels.size(); ++i) {
        buttons.emplace_back(rightX, startY + i * (buttonHeight + gapY), buttonWidth, buttonHeight, labels[i]);
    }
}

ScreenHome::~ScreenHome() {
    if(bg) { SDL_DestroyTexture(bg); bg = nullptr; }
    if(messi) { SDL_DestroyTexture(messi); messi = nullptr; }
}

void ScreenHome::render(const string& userName, bool canContinue) {
    // Draw background
    if(bg) SDL_RenderCopy(renderer, bg, NULL, NULL);
    else { SDL_SetRenderDrawColor(renderer,30,52,100,255); SDL_RenderClear(renderer); }


    // Drawing CONTINUE button if progress exists
    if (canContinue) {
        SDL_Color btncol = {255, 234, 104, 255};
        SDL_Color txtcol = {32, 40, 70, 255};
        continueButton.render(renderer, fontButton, btncol, txtcol, continueHovered, 1.13f);
    }

    // Draw other buttons
    SDL_Color btncol = {194,147,86,255};
    SDL_Color txtcol = {40,20,20,255};
    for(auto& b : buttons)
        b.render(renderer, fontButton, btncol, txtcol);
}

int ScreenHome::handleEvent(SDL_Event& e, bool canContinue) {
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    // Handle continue button if visible
    if (canContinue) {
        continueButton.setHovered(mx, my);
        continueHovered = continueButton.hovered;
        if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            if(continueButton.isClicked(mx, my)) return 5; // CONTINUE
        }
    } else {
        continueHovered = false;
    }

    // Handling other buttons
    for(size_t i=0; i<buttons.size(); ++i) {
        buttons[i].setHovered(mx, my);
        if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            if(buttons[i].isClicked(mx,my)) return static_cast<int>(i);
        }
    }
    return -1;
}