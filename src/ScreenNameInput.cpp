#include "../include/ScreenNameInput.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <ctime>

using namespace std;

ScreenNameInput::ScreenNameInput(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* fontButton, int winWidth, int winHeight)
    : renderer(renderer), font(font), fontButton(fontButton),
      windowWidth(winWidth), windowHeight(winHeight),
      inputText(""), active(true), continueHovered(false), done(false), bgTexture(nullptr)
{
    inputRect = {winWidth/2 - 255, winHeight/2 - 20, 510, 75};
    continueRect = {winWidth/2 - 123, winHeight/2 + 65, 270, 60};
    backRect = {30, 30, 56, 56}; // Size and position for back arrow

    // Load background image
    SDL_Surface* bgSurf = IMG_Load("../assets/levelselect_bg.png");
    if (bgSurf) {
        bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurf);
        SDL_FreeSurface(bgSurf);
    } else {
        bgTexture = nullptr;
        cout << "Failed to load levelselect_bg.png: " << IMG_GetError() << endl;
    }

    // Load back arrow icon
    SDL_Surface* surf = IMG_Load("../assets/back_arrow.png");
    backArrow = nullptr;
    if (surf) {
        backArrow = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
}

    // Scales and centers a rect by a given factor 
    SDL_Rect scaleRectInput(const SDL_Rect& rect, float scale) {
        int newW = static_cast<int>(rect.w * scale);
        int newH = static_cast<int>(rect.h * scale);
        int newX = rect.x - (newW - rect.w) / 2;
        int newY = rect.y - (newH - rect.h) / 2;
        return SDL_Rect{newX, newY, newW, newH};
    }


ScreenNameInput::~ScreenNameInput() {
    if (backArrow) SDL_DestroyTexture(backArrow);
    if (bgTexture) SDL_DestroyTexture(bgTexture); // free the background texture
}

void ScreenNameInput::setName(const string& name) {
    inputText = name;
}

void ScreenNameInput::render() {
    // Draw background image first
    if (bgTexture) {
        SDL_RenderCopy(renderer, bgTexture, nullptr, nullptr); // Fill window
    } else {
        SDL_SetRenderDrawColor(renderer, 32, 40, 70, 200);
        SDL_Rect bgRect = {0, 0, windowWidth, windowHeight};
        SDL_RenderFillRect(renderer, &bgRect);
    }

    // Background overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 32, 40, 70, 200);
    SDL_Rect bgRect = {0, 0, windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &bgRect);

    // Back Arrow
    if (backArrow) {
        SDL_RenderCopy(renderer, backArrow, NULL, &backRect);
    }

    // entering player name
    TTF_Font* titleFont = font;
    SDL_Color color = {255, 215, 0, 255};
    SDL_Surface* surf = TTF_RenderText_Solid(titleFont, "ENTER YOUR NAME", color);
    SDL_Texture* txt = SDL_CreateTextureFromSurface(renderer, surf);
    int tw, th;
    SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
    SDL_Rect dst = {windowWidth/2 - tw/2, windowHeight/2 - 120, tw, th};
    SDL_RenderCopy(renderer, txt, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(txt);

    // Input Box
    SDL_SetRenderDrawColor(renderer, 230, 230, 255, 255);
    SDL_RenderFillRect(renderer, &inputRect);
    SDL_SetRenderDrawColor(renderer, 120, 120, 200, 255);
    SDL_RenderDrawRect(renderer, &inputRect);

    // Render inputText + blinking cursor
    string showText = inputText;
    Uint32 ticks = SDL_GetTicks();
    bool showCursor = ((ticks / 500) % 2 == 0);
    if (active && showCursor && showText.length() < 16)
        showText += "|";
    SDL_Color inputColor = {32, 40, 70, 255};
    surf = TTF_RenderText_Solid(font, showText.c_str(), inputColor);
    txt = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
    dst = {inputRect.x + 12, inputRect.y + inputRect.h/2 - th/2, tw, th};
    SDL_RenderCopy(renderer, txt, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(txt);

            // POP-UP EFFECT for CONTINUE
    float scale = continueHovered ? 1.1f : 1.0f;
    SDL_Rect continueDraw = scaleRectInput(continueRect, scale);


    // Button fill and border
    SDL_SetRenderDrawColor(renderer, continueHovered ? 200 : 245, 205, 60, 255);
    SDL_RenderFillRect(renderer, &continueDraw);
    SDL_SetRenderDrawColor(renderer, 180, 140, 40, 255);
    SDL_RenderDrawRect(renderer, &continueDraw);

    // Draw CONTINUE label with fontButton
    SDL_Color btnCol = {32, 40, 70, 255};
    surf = TTF_RenderText_Solid(fontButton, "CONTINUE", btnCol);
    txt = SDL_CreateTextureFromSurface(renderer, surf);
    tw, th;
    SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
    dst = {
        continueDraw.x + continueDraw.w/2 - tw/2,
        continueDraw.y + continueDraw.h/2 - th/2,
        tw, th
    };
    SDL_RenderCopy(renderer, txt, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(txt);



    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

int ScreenNameInput::handleEvent(const SDL_Event& e, string& playerName) {
    // Returns 1 if continue, -1 if back, 0 otherwise

    if (e.type == SDL_MOUSEMOTION) {
        int mx = e.motion.x, my = e.motion.y;
        SDL_Point mousePoint = {mx, my};
        continueHovered = SDL_PointInRect(&mousePoint, &continueRect);
    }
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mx = e.button.x, my = e.button.y;
        SDL_Point mousePoint = {mx, my};
        if (SDL_PointInRect(&mousePoint, &continueRect)) {
            if (!inputText.empty()) {
                playerName = inputText;
                done = true;
                return 1; // CONTINUE
            }
        }
        if (SDL_PointInRect(&mousePoint, &backRect)) {
            done = false;
            return -1; // BACK
        }
    }
    if (e.type == SDL_TEXTINPUT) {
        if (inputText.length() < 16)
            inputText += e.text.text;
    }
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_BACKSPACE && !inputText.empty()) {
            inputText.pop_back();
        }
        if ((e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) && !inputText.empty()) {
            playerName = inputText;
            done = true;
            return 1; // CONTINUE
        }
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            done = false;
            return -1; // BACK
        }
    }
    return 0;
}

void ScreenNameInput::reset() {
    inputText = "";
    done = false;
}