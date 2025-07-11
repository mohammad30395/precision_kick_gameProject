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

class ScreenLevel3
{
public:
    ScreenLevel3(Game *game, SDL_Renderer *renderer, int winWidth, int winHeight);
    ~ScreenLevel3();

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
    int scrollSpeed; // 30 for level 3

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

    // Obstacles - Level 3 uses 5 types, repeating sequence
    SDL_Texture *obstacleTexture1;
    SDL_Texture *obstacleTexture2;
    SDL_Texture *obstacleTexture3;
    SDL_Texture *obstacleTexture4;
    SDL_Texture *obstacleTexture5;
    struct Obstacle {
        int type;       // 1, 2, 3, 4, or 5
        SDL_Rect rect;  // position and size
    };
    vector<Obstacle> obstacles;
    int obstacleSpeed;

    int obstaclesCleared;
    const int maxObstacles = 15;
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

    // Triple Goal System 
    SDL_Texture *goalCircle3Texture;
    SDL_Rect goalCircle3Rect;
    SDL_Rect topHoleRect;
    SDL_Rect midHoleRect;
    SDL_Rect bottomHoleRect;
    bool showGoalCircle;

    // Level complete screen
    bool levelComplete = false;
    bool congratsHomeHovered = false;
    bool congratsNextHovered = false;

    // Points System
    int currentPoints;    // Points for this run (Level 3)
    int highScore;        // High score for this player/level
    int lastGainedPoints; // Points gained on last successful run (for display)
    int lastLifeBonus;

    // Random obstacle sequence
    vector<int> obstacleSequence; // sequence of types (1,2,3,4,5)
    mt19937 rng; // For randomization - The Classic Mersenne Twister Engine

    // --- For ring scoring ---
    bool scoredTopHole;    // 300
    bool scoredMidHole;    // 200
    bool scoredBottomHole; // 100

    ScreenLevel3(const ScreenLevel3&) = delete;
    ScreenLevel3& operator=(const ScreenLevel3&) = delete;

};