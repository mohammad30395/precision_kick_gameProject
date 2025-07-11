#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <vector>
#include <random>

using namespace std;

class Game; // Forward declaration

class ScreenLevel2
{
public:
    ScreenLevel2(Game *game, SDL_Renderer *renderer, int winWidth, int winHeight);
    ~ScreenLevel2();

    void update();
    void render();
    void handleEvent(const SDL_Event &);
    void reset(bool);

    // For setting or updating the player's name from the Game class
    void setPlayerName(const string &name) { playerName = name; }
    string getPlayerName() const { return playerName; }

    int getCurrentPoints() const { return currentPoints; }
    int getHighScore() const { return highScore; }

    void setHighScore(int score) { highScore = score; }

private:
    Game *game;
    SDL_Renderer *renderer;

    // Background
    SDL_Texture *backgroundTexture;
    SDL_Rect backgroundRect;

    // Ground
    SDL_Texture *groundTexture;
    int groundWidth, groundHeight;
    SDL_Rect ground1, ground2;
    int scrollSpeed; // higher for level 2

    // Player
    SDL_Texture *playerTexture;
    int frameWidth, frameHeight, totalFrames;
    int currentFrame;
    Uint32 frameDelay;
    Uint32 lastFrameTime;

    // Jumping and gravity
    bool isJumping;
    float velocityY;
    float gravity;
    int groundY;

    // Pause/Play Button
    SDL_Texture *pauseTexture;
    SDL_Texture *playTexture;
    SDL_Rect buttonRect;

    // Obstacles - Level 2 uses 3 types, random order
    SDL_Texture *obstacleTexture1;
    SDL_Texture *obstacleTexture2;
    SDL_Texture *obstacleTexture3;
    struct Obstacle {
        int type;       // 1, 2, 3
        SDL_Rect rect;  // position and size
    };
    vector<Obstacle> obstacles;
    int obstacleSpeed;

    int obstaclesCleared;
    const int maxObstacles = 10; //Jump required is 10 to finish jumping phase
    bool jumpingPhaseOver;
    Uint32 jumpingPhaseEndTime;

    // Shooting/ball phase
    bool ballPhaseStarted;
    bool ballOnScreen;
    bool settingVelocities;

    // Ball properties
    SDL_Texture *footballTexture;
    int footballX, footballY;
    float ballX, ballY;
    float ballVX, ballVY;
    bool ballInMotion;

    // S/K keys
    bool sHeld, kHeld;
    float setVX, setVY;
    float vxBar, vyBar;
    Uint32 velocityStartTime;

    // Lives/Hearts
    int lives;
    SDL_Texture *heartTexture;

    // Attempt message
    int attemptNumber;
    Uint32 attemptMessageTime;

    SDL_Rect homeButtonRect;
    SDL_Rect retryButtonRect;
    bool homeButtonHovered;
    bool retryButtonHovered;

    bool gameOver;
    bool paused;

    int playerX, playerY;
    int windowWidth, windowHeight;

    // PAUSE MENU OVERLAY
    SDL_Rect pauseMenuRect;
    SDL_Rect pauseHomeRect;
    SDL_Rect pauseContinueRect;
    bool pauseHomeHovered;
    bool pauseContinueHovered;

    // Player Name
    string playerName;

    // --- Double Ring System ---
    SDL_Texture *doubleRingTexture; 
    SDL_Rect doubleRingRect;   // Where the PNG will be drawn
    SDL_Rect upperRingRect;
    SDL_Rect lowerRingRect;
    bool showDoubleRings;

    // Ball aiming & goal
    bool showGoalCircle;
    bool levelComplete = false;
    bool congratsHomeHovered = false;
    bool congratsNextHovered = false;

    // Points System 
    int currentPoints;    // Points for this run (Level 2)
    int highScore;        // High score for this player/level
    int lastGainedPoints; // Points gained on last successful run (for display)
    int lastLifeBonus;

    // Random obstacle sequence
    vector<int> obstacleSequence; // sequence of types (1,2,3)
    mt19937 rng; // For randomization - The Classic Mersenne Twister Engine

    // For ring scoring 
    bool scoredUpperRing; // player got upper (smaller) ring?
    bool scoredLowerRing; // player got lower (bigger) ring?

    ScreenLevel2(const ScreenLevel2&) = delete;
    ScreenLevel2& operator=(const ScreenLevel2&) = delete;

};
