#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>

using namespace std;

class Game; //Forward declaration

class ScreenLevel1
{
public:
    ScreenLevel1(Game *game, SDL_Renderer *renderer, int winWidth, int winHeight);
    ~ScreenLevel1();

    void update();
    void render();
    void handleEvent(const SDL_Event &);
    void reset();

    //For setting or updating the player's name from the Game class
    void setPlayerName(const string &name) { playerName = name; }
    string getPlayerName() const { return playerName; }

    int getCurrentPoints() const { return currentPoints; }
    int getHighScore() const { return highScore; }

    void setHighScore(int score) { highScore = score; }

private:
    Game *game;
    SDL_Renderer *renderer;

    //Background
    SDL_Texture *backgroundTexture;
    SDL_Rect backgroundRect;

    //Ground
    SDL_Texture *groundTexture;
    int groundWidth, groundHeight;
    SDL_Rect ground1, ground2;
    int scrollSpeed;

    //Player
    SDL_Texture *playerTexture;
    int frameWidth, frameHeight, totalFrames;
    int currentFrame;
    Uint32 frameDelay;
    Uint32 lastFrameTime;

    //Jumping and gravity
    bool isJumping;
    float velocityY;
    float gravity;
    int groundY;

    //Pause/Play Button
    SDL_Texture *pauseTexture;
    SDL_Texture *playTexture;
    SDL_Rect buttonRect;

    //Obstacles
    SDL_Texture *obstacleTexture;
    SDL_Rect obstacleRect;
    int obstacleSpeed;

    //Obstacle count for phase transition
    int obstaclesCleared;
    const int maxObstacles = 5;
    bool jumpingPhaseOver;
    Uint32 jumpingPhaseEndTime; //For run phase timing

    //Shooting/ball phase
    bool ballPhaseStarted;  //Ball phase (message + run to ball)
    bool ballOnScreen;      //Is ball visible?
    bool settingVelocities; //Is velocity bar input enabled?

    //Ball properties
    SDL_Texture *footballTexture;
    int footballX, footballY; //Where the ball sits (on ground)
    float ballX, ballY;       //Ball's real position (for projectile)
    float ballVX, ballVY;     //Velocity (projectile)
    bool ballInMotion;        //Ball flying after kick

    //S/K keys
    bool sHeld, kHeld;        //Key states for velocity bars
    float setVX, setVY;       //User-selected velocities
    float vxBar, vyBar;       //Bar fill values
    Uint32 velocityStartTime; //When player is allowed to set velocities

    //Lives/Hearts
    int lives;
    SDL_Texture *heartTexture;

    //Attempt message
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

    //PAUSE MENU OVERLAY
    SDL_Rect pauseMenuRect;     //Main popup rect
    SDL_Rect pauseHomeRect;     //Home button in pause popup
    SDL_Rect pauseContinueRect; //Continue button in pause popup
    bool pauseHomeHovered;      //Hover effect
    bool pauseContinueHovered;  //Hover effect

    //Player Name
    string playerName;

    //Loading goal
    SDL_Texture *goalTexture;
    SDL_Rect goalRect;
    bool showGoal; //Should be true only in ball phase

    //Goal Circle
    SDL_Texture *goalCircleTexture;
    SDL_Rect goalCircleRect;
    bool showGoalCircle;

    bool levelComplete = false;

    bool congratsHomeHovered = false;
    bool congratsNextHovered = false;

    //Points System
    int currentPoints;    //points for this run (this attempt)
    int highScore;        //high score for this player and level
    int lastGainedPoints; //points gained on last successful run (for display at GAME OVER or CONGRATULATIONS screen)

    int lastLifeBonus;
};
