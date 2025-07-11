#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

using namespace std;

class Button {
public:
    SDL_Rect rect;
    string text;
    bool hovered = false;

    Button(int x, int y, int w, int h, const string& t);

    void render(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color, SDL_Color textColor);

    // render with pop up effect 
    void render(SDL_Renderer* renderer, TTF_Font* font, SDL_Color color, SDL_Color textColor, bool pop, float popScale = 1.13f);

    bool isClicked(int mx, int my);
    void setHovered(int mx, int my);
};