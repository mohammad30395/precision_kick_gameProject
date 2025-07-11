#include "../include/ScreenCredits.h"
#include <SDL2/SDL_image.h>
#include <iostream>

using namespace std;

ScreenCredits::ScreenCredits(SDL_Renderer* renderer, TTF_Font* fontTitle, TTF_Font* fontButton, int winWidth, int winHeight)
    : renderer(renderer), fontTitle(fontTitle), fontButton(fontButton),
      windowWidth(winWidth), windowHeight(winHeight), backPressed(false), soundOn(true)
{
    // Loading textures
    bgTexture = loadTexture("../assets/levelselect_bg.png");
    backBtnTexture = loadTexture("../assets/back_arrow.png");
    soundOnTexture = loadTexture("../assets/music_on.png");
    soundOffTexture = loadTexture("../assets/music_off.png");

    // Button locations and sizes
    int btnSize = 64;
    int margin = 40;
    backBtnRect = { margin, margin, btnSize, btnSize };
    soundBtnRect = { windowWidth - btnSize - margin, margin, btnSize, btnSize };

    // Shadow box for credits text
    int boxMarginX = 100, boxMarginY = 120;
    textBoxRect = { boxMarginX, boxMarginY, windowWidth - 2 * boxMarginX, windowHeight - 2 * boxMarginY };

    // Credits text
    creditsText =
        "CREDITS\n"
        "Project Team:\n"
        "* Shashwata Nandi\n"
        "* Sudipto Debnath\n"
        "* Swarlok Samadder\n"
        "* Mohammad Mahmudul Kabir Fahmid\n"
        "Images:                  Font:\n"
        "* GettyImages      * Audiowide Regular\n"
        "* FreePik\n"
        "Music:\n"
        "* Magic in The Air by Magic System\n";
}

ScreenCredits::~ScreenCredits() {
    if (bgTexture) SDL_DestroyTexture(bgTexture);
    if (backBtnTexture) SDL_DestroyTexture(backBtnTexture);
    if (soundOnTexture) SDL_DestroyTexture(soundOnTexture);
    if (soundOffTexture) SDL_DestroyTexture(soundOffTexture);
}

SDL_Texture* ScreenCredits::loadTexture(const string& path) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        cerr << "IMG_Load Error: " << IMG_GetError() << " (" << path << ")\n";
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

void ScreenCredits::drawShadowBox(SDL_Rect rect, SDL_Color color, int alpha) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

SDL_Texture* ScreenCredits::createWrappedText(const string& text, SDL_Color color, int maxWidth) {
    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(fontButton, text.c_str(), color, maxWidth);
    SDL_Texture* texture = nullptr;
    if (surface) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    return texture;
}

bool ScreenCredits::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;
        // Back button
        if (mx >= backBtnRect.x && mx <= backBtnRect.x + backBtnRect.w &&
            my >= backBtnRect.y && my <= backBtnRect.y + backBtnRect.h)
        {
            backPressed = true;
        }
        // Sound button
        else if (mx >= soundBtnRect.x && mx <= soundBtnRect.x + soundBtnRect.w &&
                 my >= soundBtnRect.y && my <= soundBtnRect.y + soundBtnRect.h)
        {
            soundOn = !soundOn;
        }
    }
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            backPressed = true;
        }
    }
    bool ret = backPressed;
    backPressed = false;
    return ret;
}

void ScreenCredits::render() {
    // Draw background
    if (bgTexture) SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
    else { SDL_SetRenderDrawColor(renderer, 30, 52, 100, 255); SDL_RenderClear(renderer); }

    // Draw shadowed text box
    drawShadowBox(textBoxRect, {0, 0, 0}, 160);

    // Draw the credits text (centered in box)
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Texture* txtTexture = createWrappedText(creditsText, textColor, textBoxRect.w - 40);

    if (txtTexture) {
        int tw = 0, th = 0;
        SDL_QueryTexture(txtTexture, NULL, NULL, &tw, &th);
        int textX = textBoxRect.x + (textBoxRect.w - tw) / 2;
        int textY = textBoxRect.y + 30;
        SDL_Rect dstRect = { textX, textY, tw, th };
        SDL_RenderCopy(renderer, txtTexture, NULL, &dstRect);
        SDL_DestroyTexture(txtTexture);
    }

    // Draw BACK button
    if (backBtnTexture) SDL_RenderCopy(renderer, backBtnTexture, NULL, &backBtnRect);
}