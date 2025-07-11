#include "../include/Button.h"

Button::Button(int x, int y, int w, int h, const std::string& t)
    : rect{x, y, w, h}, text(t) {}

void Button::render(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color, SDL_Color textColor) {
    // Animation:- enlarge button if hovered
    SDL_Rect drawRect = rect;
    if (hovered) {
        int inflateW = rect.w * 0.12;  // 12% wider
        int inflateH = rect.h * 0.15;  // 15% taller
        drawRect.x -= inflateW / 2;
        drawRect.y -= inflateH / 2;
        drawRect.w += inflateW;
        drawRect.h += inflateH;
    }

    // Draw button background and border
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &drawRect);
    SDL_SetRenderDrawColor(renderer, 40, 20, 20, 255);
    SDL_RenderDrawRect(renderer, &drawRect);

    // Draw text centered in the button
    if (font) {
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {
            drawRect.x + drawRect.w/2 - textSurface->w/2,
            drawRect.y + drawRect.h/2 - textSurface->h/2,
            textSurface->w, textSurface->h
        };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

void Button::render(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color, SDL_Color textColor, bool pop, float popScale) {
    // Using scale if pop is true, else normal
    SDL_Rect drawRect = rect;
    if (pop) {
        int newW = rect.w * popScale;
        int newH = rect.h * popScale;
        drawRect.x = rect.x - (newW - rect.w) / 2;
        drawRect.y = rect.y - (newH - rect.h) / 2;
        drawRect.w = newW;
        drawRect.h = newH;
    }
    // Draw button background and border
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &drawRect);
    SDL_SetRenderDrawColor(renderer, 40, 20, 20, 255);
    SDL_RenderDrawRect(renderer, &drawRect);

    // Draw text centered in the button
    if (font) {
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {
            drawRect.x + drawRect.w/2 - textSurface->w/2,
            drawRect.y + drawRect.h/2 - textSurface->h/2,
            textSurface->w, textSurface->h
        };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}


bool Button::isClicked(int mx, int my) {
    return mx >= rect.x && mx <= rect.x + rect.w && my >= rect.y && my <= rect.y + rect.h;
}

void Button::setHovered(int mx, int my) {
    hovered = isClicked(mx, my);
}