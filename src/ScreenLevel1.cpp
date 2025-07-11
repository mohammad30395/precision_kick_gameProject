#include "../include/ScreenLevel1.h"
#include "../include/Game.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>

using namespace std;

//scaleRect for popping up buttons when hovering, uses scale variable for increasing their size(popping up)
SDL_Rect scaleRect(const SDL_Rect &rect, float scale)
{
    int newW = static_cast<int>(rect.w * scale);
    int newH = static_cast<int>(rect.h * scale);
    int newX = rect.x - (newW - rect.w) / 2;
    int newY = rect.y - (newH - rect.h) / 2;
    return SDL_Rect{newX, newY, newW, newH};
}

ScreenLevel1::ScreenLevel1(Game *game, SDL_Renderer *renderer, int winWidth, int winHeight) //This is constructor. Initial value set here!
    : game(game), renderer(renderer),
      backgroundTexture(nullptr), groundTexture(nullptr), playerTexture(nullptr),
      groundWidth(64), groundHeight(0),
      frameWidth(96), frameHeight(96), totalFrames(10), currentFrame(0),
      frameDelay(100), lastFrameTime(SDL_GetTicks()),
      scrollSpeed(23)/*speed can be set here*/, isJumping(false), velocityY(0.0f), gravity(1.0f)/*gravity can be set here*/,
      groundY(0), pauseTexture(nullptr), playTexture(nullptr),
      obstacleTexture(nullptr), obstacleSpeed(0),
      playerX(100), playerY(0), windowWidth(winWidth), windowHeight(winHeight),
      homeButtonHovered(false), retryButtonHovered(false), lives(3), heartTexture(nullptr),
      attemptNumber(0), attemptMessageTime(0),
      obstaclesCleared(0), jumpingPhaseOver(false), jumpingPhaseEndTime(0),
      ballPhaseStarted(false), ballOnScreen(false), settingVelocities(false),
      footballTexture(nullptr), footballX(0), footballY(0),
      ballX(0), ballY(0), ballVX(0), ballVY(0), ballInMotion(false),
      sHeld(false), kHeld(false), setVX(0), setVY(0), vxBar(0), vyBar(0), velocityStartTime(0),
      gameOver(false), paused(false),
      pauseHomeHovered(false), pauseContinueHovered(false), playerName(""),
      currentPoints(0), highScore(0), lastGainedPoints(0), lastLifeBonus(0)

{

    goalCircleTexture = IMG_LoadTexture(renderer, "../assets/goal_circle.png");
    if (!goalCircleTexture)
        cout << "Failed to load goal circle: " << IMG_GetError() << endl;
    showGoalCircle = false;

    //Background Texture Load
    backgroundTexture = IMG_LoadTexture(renderer, "../assets/level1_stadium.png");
    if (!backgroundTexture)
        cout << "Failed to load background: " << IMG_GetError() << endl;
    backgroundRect = {0, 0, 1600, 664};

    //Ground
    groundTexture = IMG_LoadTexture(renderer, "../assets/level1_ground.png");
    if (!groundTexture)
        cout << "Failed to load ground: " << IMG_GetError() << endl;
    SDL_QueryTexture(groundTexture, NULL, NULL, &groundWidth, &groundHeight);

    ground1 = {0, windowHeight - groundHeight + 70, groundWidth, groundHeight};
    ground2 = {groundWidth, windowHeight - groundHeight + 70, groundWidth, groundHeight};

    //Player
    playerTexture = IMG_LoadTexture(renderer, "../assets/sprite1.png");
    if (!playerTexture)
        cout << "Failed to load player sprite: " << IMG_GetError() << endl;

    playerY = windowHeight - groundHeight - frameHeight + 70;
    groundY = playerY;

    //Pause and Play buttons
    pauseTexture = IMG_LoadTexture(renderer, "../assets/pause.png");
    playTexture = IMG_LoadTexture(renderer, "../assets/play.png");
    buttonRect = {windowWidth - 80, 20, 64, 64};

    //Obstacle
    obstacleTexture = IMG_LoadTexture(renderer, "../assets/obstacle.png");
    if (!obstacleTexture)
        cout << "Failed to load obstacle: " << IMG_GetError() << endl;
    obstacleRect = {windowWidth + 200, ground1.y - 50 + 20, 60, 100};
    obstacleSpeed = scrollSpeed;

    //Overlay buttons
    homeButtonRect = {winWidth / 2 - 120, winHeight / 2 + 40, 100, 50};
    retryButtonRect = {winWidth / 2 + 20, winHeight / 2 + 40, 150, 50};

    //Life/Heart
    heartTexture = IMG_LoadTexture(renderer, "../assets/heart.png");
    if (!heartTexture)
        cout << "Failed to load heart: " << IMG_GetError() << endl;

    //Ball
    footballTexture = IMG_LoadTexture(renderer, "../assets/football.png");
    if (!footballTexture)
        cout << "Failed to load football: " << IMG_GetError() << endl;

    //Pause menu rect
    pauseMenuRect = {winWidth / 2 - 200, winHeight / 2 - 130, 450, 260};
    pauseHomeRect = {winWidth / 2 - 150, winHeight / 2 + 20, 130, 60};
    pauseContinueRect = {winWidth / 2 + 30, winHeight / 2 + 20, 200, 60};
    pauseHomeHovered = pauseContinueHovered = false;

    reset();
}

ScreenLevel1::~ScreenLevel1() //Destructor for all the textures in contsructor above - Memory leak is avoided
{
    if (backgroundTexture)
        SDL_DestroyTexture(backgroundTexture);
    if (groundTexture)
        SDL_DestroyTexture(groundTexture);
    if (playerTexture)
        SDL_DestroyTexture(playerTexture);
    if (pauseTexture)
        SDL_DestroyTexture(pauseTexture);
    if (playTexture)
        SDL_DestroyTexture(playTexture);
    if (obstacleTexture)
        SDL_DestroyTexture(obstacleTexture);
    if (heartTexture)
        SDL_DestroyTexture(heartTexture);
    if (footballTexture)
        SDL_DestroyTexture(footballTexture);
}

/*All game logic mainly inside update. All logics like obstacles collision, jump, jump motion, velocity bar's appearance, velocity set, football
's appearance, shooting mechanism(collision of the player's rect and football's rect), football's projectile motion, football's collision with ring
hole's rect - everything is coded in this section*/

void ScreenLevel1::update() 
{

    //levelComplete will result in return, no update
    if (levelComplete)
        return;

    if (paused || gameOver)
        return;

    //Jumping physics - gravity is used here - only changes velocityY
    if (isJumping)
    {
        playerY += static_cast<int>(velocityY);
        velocityY += gravity; //vertical velocity useing gravity
        if (playerY >= groundY)
        {
            playerY = groundY;//takes back to ground when playerY is larger than groundY
            isJumping = false;//jumping boolean back to false as jumping phase is over
            velocityY = 0.0f; //falling to ground makes the velocityY back to 0
        }
    }

    //Animation of player
    if (SDL_GetTicks() - lastFrameTime >= frameDelay) //SDL_GetTicks() gets the number of miliseconds, simply speaking time since SDL_Init()
    {
        currentFrame = (currentFrame + 1) % totalFrames; //Sprite sheet animation formula
        lastFrameTime = SDL_GetTicks();
    }

    //Scroll ground towards left
    ground1.x -= scrollSpeed;
    ground2.x -= scrollSpeed;
    if (ground1.x + groundWidth <= 0)
        ground1.x = ground2.x + groundWidth; //wraps up(begins from the beginning) if it goes offscreen
    if (ground2.x + groundWidth <= 0)
        ground2.x = ground1.x + groundWidth; //wraps up(begins from the beginning) if it goes offscreen

    //Scroll goal_circle (ring scrolling)
    if (showGoalCircle && !ballInMotion)
    {
        goalCircleRect.x -= scrollSpeed;
    }

    //Scroll the goal with the ground (ring hole scrolling)
    if (showGoalCircle && !ballInMotion)
    {
        goalRect.x -= scrollSpeed;
    }

    //PHASE 1
    //OBSTACLE JUMPING

    if (!jumpingPhaseOver)
    {
        //Moves obstacle
        obstacleRect.x -= obstacleSpeed; //obstacle's rectangle texture moves using obstacle speeed
        if (obstacleRect.x + obstacleRect.w < 0)
        {
            //If player jumps over an obstacle
            obstaclesCleared++;
            currentPoints += 25; //25 Points per obstacle cleared(jumped)

            if (obstaclesCleared < maxObstacles)
            {
                //Respawns the next obstacle
                obstacleRect.x = windowWidth + rand() % 200; //rand() % 200 means obstacles respawn maximum 199 pixels off the right edge of the screen
            }
            else
            {   //all variables set like this up until jumping phase is over and player clears all obstacles
                jumpingPhaseOver = true;
                jumpingPhaseEndTime = SDL_GetTicks(); //used in future for counting 5 seconds after jumping phase ends
                ballPhaseStarted = false;
                ballOnScreen = false; 
                settingVelocities = true;
                velocityStartTime = SDL_GetTicks();
                vxBar = vyBar = 0;
                setVX = setVY = 0;
                sHeld = kHeld = false;
                playerX = 100; //player's intial x-coordinate in this phase
                playerY = groundY;
            }
        }

        //Obstacle collision
        SDL_Rect playerRect = {playerX, playerY - 15, frameWidth + 40, frameHeight + 100}; /*player's rect that is being used as our player behind that
        sprite sheet*/

        if (SDL_HasIntersection(&playerRect, &obstacleRect)) //Collision checking function
        {
            if (lives > 1)
            {
                lives--;
                attemptNumber = 4 - lives; //2 or 3
                attemptMessageTime = SDL_GetTicks();
                playerY = groundY;
                isJumping = false;
                velocityY = 0.0f;
                obstacleRect.x = windowWidth + rand() % 200; //obstacleRect.x respawn logic as above
                obstaclesCleared = 0; //One fail will reset back the jumped obstacle count to 0
                currentPoints = 0;    //Fail also makes point back to 0
                lastGainedPoints = 0; //Same logic as above
            }
            else
            {   //gameOver makes all these varibales, life becomes 0 unless retry is pressed and point count set to 0
                lives = 0;
                gameOver = true;
                currentPoints = 0;
                lastGainedPoints = 0;
            }
        }
        return; //if it is still in obstacle phase update() ends here
    }

    //PHASE 2 //5-Second Run With Velocity Bars
    // After obstacle phase is successfully done, gamer gets 2.7 seconds to set his vertical and horizonal velocities to shoot his shot

    Uint32 timeSinceJumpingOver = SDL_GetTicks() - jumpingPhaseEndTime;
    if (!ballPhaseStarted && timeSinceJumpingOver <= 2700)
    {
        //Allow S/K to fill bars
        if (settingVelocities)
        {
            if (sHeld)
                vyBar = min(1.0f, vyBar + 0.028f); //vertical velocity bar increases as S key is pressed
            if (kHeld)
                vxBar = min(1.0f, vxBar + 0.028f); //horizontal velocity bar increases as K key is pressed
        }
        //Scroll ground and player (no ball on screen yet)
        playerX -= scrollSpeed;
        if (playerX < 100)
            playerX = 100; //wraps up if off screen
        return;
    }

    //After 2 seconds, ballPhase starts and player runs towards automatically using the velocity set and kicks the ball automatically
    if (!ballPhaseStarted && timeSinceJumpingOver > 2700)
    {
        ballPhaseStarted = true;
        ballOnScreen = true;
        settingVelocities = false;

        //Set the ball's initial position
        footballX = windowWidth;
        footballY = groundY + frameHeight;
        ballX = footballX;
        ballY = footballY;

        //The goal circle position is at a fixed distance in front of the ball
        int goalGap = 600; //gap between ball and goal circle, can be changed to increase or decrease the gap
        goalCircleRect = {(int)ballX + goalGap, (int)ballY - 250, 400, 400}; 

        showGoalCircle = true; //Shows the goal circle now

        //Rectangle for goal detection(Our actual target/hole)
        goalRect = {
            goalCircleRect.x + goalCircleRect.w / 2 - 55,  //center of goal_circle minus half width - 55 for preper positioning
            goalCircleRect.y + goalCircleRect.h / 2 - 120, //center of goal_circle minus half height - 120 for proper positioning
            80,                                            //width of rectangle
            150                                            //height of rectangle
        };
    }

    //If ball phase hasn't started, return
    if (!ballPhaseStarted)
        return;

    //Ball has appeared but not in motion
    if (ballOnScreen && !ballInMotion)
        ballX -= scrollSpeed; //Similar to ground or obstacle scrolling logic

    //Ball scrolls with ground until player reaches it
    //Player automatically runs toward the ball when appears until the kick
    if (ballOnScreen && !ballInMotion)
    {
        //Ball moves left
        if (playerX + frameWidth + 30 < ballX)
            playerX += max(2, scrollSpeed / 4);
        else
        {
            //Ball kicking
            if (vxBar == 0 && vyBar == 0)
            {
                //If no velocity is set, automatically fails and loses life or game is over if no life left
                if (lives > 1)
                {
                    lives--; //life is lost if player fails
                    attemptNumber = 4 - lives; //max 3 attempts
                    attemptMessageTime = SDL_GetTicks(); //used to show ATTEMPT 2 or ATTEMPT 3 for a while
                    reset();
                    return;
                }
                else
                {
                    lives = 0;
                    gameOver = true;
                    return;
                }
            }
            //Shooting 
            ballInMotion = true;
            setVX = vxBar; //Horizontal bar's velocity used for ball's horizontal velocity
            setVY = vyBar; //Vertical bar's velocity used to ball's vertical velocity
            ballVX = max(5.0f, vxBar * 40.0f);    //minimum 5.0 ballVX whether or not vxBar has any value; vxBar * 40.0f converts vxBar to ballVX
            ballVY = max(-20.0f, -vyBar * 45.0f); //minimum-20.0 ballVY whether or not vyBar has any value; -vyBar * 45.0f converts vyBar to ballVY
        }
    }

    //Ball movement after kick
    //---PROJECTILE MOTION---

    if (ballInMotion)
    {
        ballX += ballVX; //Move horizontally by the ball's horizontal speed
        ballY += ballVY; //Move vertically by the ball's vertical speed
        ballVY += 0.6f;  //Gravity increases vertical speed (downwards) each frame

        /*So, what basically happens here is the ballVX stays constant but ballVY at first is negative. That means the ball is going upwards.
        However, as each frame passes gravity is added to ballVY and at a stage its speed slows down and stops for a while, meaning the ballVY
        becomes 0 or close to 0. Then, its motion is downward again and the ballVY keeps increasing due to the addition of gravity. The motion
        traces out a parabolic path which indicates it's a PROJECTILE MOTION.*/

        //Ball entering hole check - collision with goalRect
        SDL_Rect ballRect = {(int)ballX, (int)ballY, 64, 64};
        if (showGoalCircle && SDL_HasIntersection(&ballRect, &goalRect))
        {
            //Variables when ball enters hole(Level complete)
            showGoalCircle = false;
            ballInMotion = false;
            levelComplete = true;
            game->setLevelCompleted(1); //Gamestate changes here
            cout << "SUCCESS! Ball entered the rectangle hole." << endl; //Debug code. Output will be in terminal.
            int bonus = lives * 50;   //50 per life left
            int gained = 100 + bonus; //100 for goal, plus life bonus
            currentPoints += gained; //Will be used to store points in all levels
            lastGainedPoints = gained; //Display in congrats overlay
            lastLifeBonus = bonus;

            //If currentPoints is higher than previous highScore, then it will be replaced
            if (currentPoints > highScore)
            {
                highScore = currentPoints;
                game->setScoreForPlayer(playerName, highScore, 1); //Assigns the highScore to the input playerName
            }

            return;//No more ball movement/checks, just return
        }

        //Fail condition checks (Ball falls on ground or goes out of bound(does not enter the hole))
        int groundLevel = footballY;
        if (ballY >= groundLevel || ballX > windowWidth || ballX < -64) //ball is on or below ground level or far right off the screen or far left off the screen
        {
            ballInMotion = false;
            currentPoints = 0; 
            lastGainedPoints = 0;
            if (lives > 1)
            {
                lives--;
                attemptNumber = 4 - lives;
                attemptMessageTime = SDL_GetTicks();
                reset();
            }
            else
            {
                lives = 0;
                gameOver = true;
            }
            return;
        }
    }
}

void ScreenLevel1::render()
{
    //Background
    if (backgroundTexture)
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &backgroundRect);

    //Ground
    if (groundTexture)
    {
        SDL_RenderCopy(renderer, groundTexture, NULL, &ground1);
        SDL_RenderCopy(renderer, groundTexture, NULL, &ground2);
    }

    //Goal circle
    if (showGoalCircle && goalCircleTexture)
        SDL_RenderCopy(renderer, goalCircleTexture, NULL, &goalCircleRect);

    /*---DEBUG CODE---
    if (showGoalCircle)
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 180); // Green, semi-transparent
        SDL_RenderDrawRect(renderer, &goalRect);
    }*/

    //The ball's center (magenta) when on screen, barely visible - origially for debugging
    if (ballOnScreen || ballInMotion)
    {
        int ballCenterX = (int)ballX + 32;
        int ballCenterY = (int)ballY + 32;
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); // Magenta
        SDL_Rect dotRect = {ballCenterX - 2, ballCenterY - 2, 5, 5};
        SDL_RenderFillRect(renderer, &dotRect);
    }

    //Lives as hearts
    if (heartTexture)
    {
        int heartW = 40, heartH = 40, spacing = 8;
        for (int i = 0; i < lives; ++i)
        {
            SDL_Rect dst = {10 + i * (heartW + spacing), 10, heartW, heartH};
            SDL_RenderCopy(renderer, heartTexture, NULL, &dst);
        }
    }

    //Current points below hearts
    if (!gameOver)
    {
        TTF_Font *scoreFont = TTF_OpenFont("../assets/font.ttf", 32);
        if (scoreFont)
        {
            SDL_Color scoreColor = {255, 255, 255, 255};
            string scoreStr = "Points: " + to_string(currentPoints);
            SDL_Surface *scoreSurf = TTF_RenderText_Solid(scoreFont, scoreStr.c_str(), scoreColor);
            if (scoreSurf)
            {
                SDL_Texture *scoreTex = SDL_CreateTextureFromSurface(renderer, scoreSurf);
                int sw = 0, sh = 0;
                SDL_QueryTexture(scoreTex, NULL, NULL, &sw, &sh);
                SDL_Rect scoreDst = {40, 60, sw, sh};
                SDL_RenderCopy(renderer, scoreTex, NULL, &scoreDst);
                SDL_FreeSurface(scoreSurf);
                SDL_DestroyTexture(scoreTex);
            }
            TTF_CloseFont(scoreFont);
        }
    }

    //The player name at top center of the screen
    if (!playerName.empty())
    {
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 36);
        if (font)
        {
            SDL_Color color = {255, 215, 0, 255}; //Gold/yellow
            SDL_Surface *surf = TTF_RenderText_Solid(font, playerName.c_str(), color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw, th;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {windowWidth / 2 - tw / 2, 18, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(font);
        }
    }

    //ATTEMPT message appearance when life is lost
    if (attemptNumber >= 2 && SDL_GetTicks() - attemptMessageTime < 1200)
    {
        string msg = "ATTEMPT " + to_string(attemptNumber);
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 56);
        if (font)
        {
            SDL_Color color = {255, 215, 0, 255}; //Gold/yellow
            SDL_Surface *surf = TTF_RenderText_Solid(font, msg.c_str(), color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw = 0, th = 0;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {windowWidth / 2 - tw / 2, windowHeight / 2 - th / 2 - 100, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(font);
        }
    }

    //Obstacle
    if (!jumpingPhaseOver && obstacleTexture && !gameOver)
    {
        SDL_RenderCopy(renderer, obstacleTexture, NULL, &obstacleRect);
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 36);
        if (font)
        {
            string counter = to_string(max(0, maxObstacles - obstaclesCleared)) + " left"; /*Shows how many obs left on right of the screen below pause 
            button by subtracting the obstaclesCleared from maxObstacles*/
            SDL_Color color = {0, 0, 0, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(font, counter.c_str(), color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw = 0, th = 0;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {windowWidth - tw - 40, 60, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(font);
        }
    }

    //Player
    if (playerTexture && !gameOver)
    {
        SDL_Rect srcRect = {currentFrame * frameWidth, 0, frameWidth, frameHeight};
        SDL_Rect destRect = {playerX, playerY - 15, frameWidth + 40, frameHeight + 100};
        SDL_RenderCopy(renderer, playerTexture, &srcRect, &destRect);
        //Takes the srcRect(one frame of the sprite and puts it in destRect or basically scales it and keeps doing frame by frame
        //Sprite sheet animation
    }

    //Football
    if (ballOnScreen || ballInMotion)
    {
        if (footballTexture)
        {
            SDL_Rect ballDst = {(int)ballX, (int)ballY, 64, 64};
            SDL_RenderCopy(renderer, footballTexture, NULL, &ballDst);
        }
    }

    //Velocity bar that shows only for 2.7~3 seconds
    Uint32 timeSinceJumpingOver = SDL_GetTicks() - jumpingPhaseEndTime;
    if (jumpingPhaseOver && !ballPhaseStarted && timeSinceJumpingOver <= 2700)
    {
        //Bar background
        SDL_Rect vxBarBG = {windowWidth / 2 - 200, windowHeight - 70, 180, 24};
        SDL_Rect vyBarBG = {windowWidth / 2 + 20, windowHeight - 70, 180, 24};
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderFillRect(renderer, &vxBarBG);
        SDL_RenderFillRect(renderer, &vyBarBG);

        //Bar fill
        SDL_SetRenderDrawColor(renderer, 20, 200, 50, 255);
        SDL_Rect vxFill = vxBarBG; 
        vxFill.w = (int)(vxBar * vxBarBG.w); //Holding K key increases the bar
        SDL_Rect vyFill = vyBarBG; 
        vyFill.w = (int)(vyBar * vyBarBG.w); //Holding S key increases the bar
        SDL_RenderFillRect(renderer, &vxFill);
        SDL_RenderFillRect(renderer, &vyFill);

        //Labels for the bars
        TTF_Font *lblFont = TTF_OpenFont("../assets/font.ttf", 20);
        if (lblFont)
        {
            SDL_Color color = {255, 255, 255, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(lblFont, "HORIZONTAL", color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw = 0, th = 0;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th); 
                SDL_Rect dst = {vxBarBG.x + vxBarBG.w / 2 - tw / 2, vxBarBG.y - 26, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            SDL_Surface *surf2 = TTF_RenderText_Solid(lblFont, "VERTICAL", color);
            if (surf2)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf2);
                int tw = 0, th = 0;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th); 
                SDL_Rect dst = {vyBarBG.x + vyBarBG.w / 2 - tw / 2, vyBarBG.y - 26, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf2);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(lblFont);
        }
    }

    //Pause/play button
    if (paused)
    {
        if (playTexture)
            SDL_RenderCopy(renderer, playTexture, NULL, &buttonRect);
    }
    else
    {
        if (pauseTexture)
            SDL_RenderCopy(renderer, pauseTexture, NULL, &buttonRect);
    }

    //Pause Menu
    if (paused)
    {
        //Dim background
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 180);
        SDL_RenderFillRect(renderer, &pauseMenuRect);

        //PAUSED text
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 52);
        if (font)
        {
            SDL_Color color = {255, 215, 0, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(font, "PAUSED", color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw, th;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {pauseMenuRect.x + pauseMenuRect.w / 2 - tw / 2, pauseMenuRect.y + 40, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(font);
        }

        //HOME button render
        SDL_Rect homeDraw = pauseHomeHovered ? scaleRect(pauseHomeRect, 1.11f) : pauseHomeRect;
        SDL_SetRenderDrawColor(renderer, 245, 210, 40, 255);
        SDL_RenderFillRect(renderer, &homeDraw);
        if (pauseHomeHovered)
        {
            SDL_SetRenderDrawColor(renderer, 180, 140, 40, 255);
            SDL_RenderDrawRect(renderer, &homeDraw);
        }
        //CONTINUE button render
        SDL_Rect contDraw = pauseContinueHovered ? scaleRect(pauseContinueRect, 1.11f) : pauseContinueRect;
        SDL_SetRenderDrawColor(renderer, 60, 220, 140, 255);
        SDL_RenderFillRect(renderer, &contDraw);
        if (pauseContinueHovered)
        {
            SDL_SetRenderDrawColor(renderer, 30, 140, 80, 255);
            SDL_RenderDrawRect(renderer, &contDraw);
        }

        //Button labels
        TTF_Font *btnFont = TTF_OpenFont("../assets/font.ttf", 32);
        if (btnFont)
        {
            SDL_Color btnColor = {30, 30, 30, 255};
            //HOME label
            SDL_Surface *homeSurf = TTF_RenderText_Solid(btnFont, "HOME", btnColor);
            if (homeSurf)
            {
                SDL_Texture *homeTxt = SDL_CreateTextureFromSurface(renderer, homeSurf);
                int tw, th;
                SDL_QueryTexture(homeTxt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {homeDraw.x + homeDraw.w / 2 - tw / 2, homeDraw.y + homeDraw.h / 2 - th / 2, tw, th};
                SDL_RenderCopy(renderer, homeTxt, NULL, &dst);
                SDL_FreeSurface(homeSurf);
                SDL_DestroyTexture(homeTxt);
            }
            //CONTINUE label
            SDL_Surface *contSurf = TTF_RenderText_Solid(btnFont, "CONTINUE", btnColor);
            if (contSurf)
            {
                SDL_Texture *contTxt = SDL_CreateTextureFromSurface(renderer, contSurf);
                int tw, th;
                SDL_QueryTexture(contTxt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {contDraw.x + contDraw.w / 2 - tw / 2, contDraw.y + contDraw.h / 2 - th / 2, tw, th};
                SDL_RenderCopy(renderer, contTxt, NULL, &dst);
                SDL_FreeSurface(contSurf);
                SDL_DestroyTexture(contTxt);
            }
            TTF_CloseFont(btnFont);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        return; //PAUSED so UI option not accessible
    }

    //GAME OVER overlay
    if (gameOver)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
        SDL_Rect overlayRect = {0, 0, windowWidth, windowHeight};
        SDL_RenderFillRect(renderer, &overlayRect);

        // GAME OVER text
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 48);
        if (font)
        {
            SDL_Color color = {255, 0, 0, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(font, "GAME OVER", color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw = 0, th = 0;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {windowWidth / 2 - tw / 2 + 32, windowHeight / 2 - 80, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(font);
            //Showing Points/High Score
            TTF_Font *pointsFont = TTF_OpenFont("../assets/font.ttf", 40);
            if (pointsFont)
            {
                SDL_Color color = {255, 255, 255, 255};
                string pointsMsg = "Points: " + to_string(currentPoints);
                SDL_Surface *surf = TTF_RenderText_Solid(pointsFont, pointsMsg.c_str(), color);
                if (surf)
                {
                    SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                    int tw = 0, th = 0;
                    SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                    //This is shown below the GAME OVER text
                    SDL_Rect dst = {windowWidth / 2 - tw / 2 + 32, windowHeight / 2 - 20, tw, th};
                    SDL_RenderCopy(renderer, txt, NULL, &dst);
                    SDL_FreeSurface(surf);
                    SDL_DestroyTexture(txt);
                }
                TTF_CloseFont(pointsFont);
            }
        }

        //Pop-Up buttons
        SDL_Rect homeRectDraw = homeButtonHovered ? scaleRect(homeButtonRect, 1.1f) : homeButtonRect;
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        SDL_RenderFillRect(renderer, &homeRectDraw);
        if (homeButtonHovered)
        {
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            SDL_RenderDrawRect(renderer, &homeRectDraw);
        }

        TTF_Font *btnFont = TTF_OpenFont("../assets/font.ttf", 28);
        if (btnFont)
        {
            SDL_Color btnColor = {0, 0, 0, 255};
            SDL_Surface *homeSurf = TTF_RenderText_Solid(btnFont, "HOME", btnColor);
            if (homeSurf)
            {
                SDL_Texture *homeTxt = SDL_CreateTextureFromSurface(renderer, homeSurf);
                int tw = 0, th = 0;
                SDL_QueryTexture(homeTxt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {
                    homeRectDraw.x + homeRectDraw.w / 2 - tw / 2,
                    homeRectDraw.y + homeRectDraw.h / 2 - th / 2,
                    tw, th};
                SDL_RenderCopy(renderer, homeTxt, NULL, &dst);
                SDL_FreeSurface(homeSurf);
                SDL_DestroyTexture(homeTxt);
            }

            //RETRY button
            SDL_Rect retryRectDraw = retryButtonHovered ? scaleRect(retryButtonRect, 1.1f) : retryButtonRect;
            SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
            SDL_RenderFillRect(renderer, &retryRectDraw);
            if (retryButtonHovered)
            {
                SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
                SDL_RenderDrawRect(renderer, &retryRectDraw);
            }

            SDL_Surface *retrySurf = TTF_RenderText_Solid(btnFont, "RETRY", btnColor);
            if (retrySurf)
            {
                SDL_Texture *retryTxt = SDL_CreateTextureFromSurface(renderer, retrySurf);
                int tw = 0, th = 0;
                SDL_QueryTexture(retryTxt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {
                    retryRectDraw.x + retryRectDraw.w / 2 - tw / 2,
                    retryRectDraw.y + retryRectDraw.h / 2 - th / 2,
                    tw, th};
                SDL_RenderCopy(renderer, retryTxt, NULL, &dst);
                SDL_FreeSurface(retrySurf);
                SDL_DestroyTexture(retryTxt);
            }
            TTF_CloseFont(btnFont);
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    if (levelComplete)
    {
        //Dim background
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 180);
        SDL_Rect overlayRect = {0, 0, windowWidth, windowHeight};
        SDL_RenderFillRect(renderer, &overlayRect);

        //Congratulations message
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 52);
        if (font)
        {
            SDL_Color color = {255, 215, 0, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(font, "CONGRATULATIONS!", color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw, th;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {windowWidth / 2 - tw / 2, windowHeight / 2 - th / 2 - 80, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(font);
        }

        //Show points earned this level
        TTF_Font *scoreFont = TTF_OpenFont("../assets/font.ttf", 40);
        if (scoreFont)
        {
            SDL_Color color = {255, 255, 255, 255};
            string ptsMsg = "Points: " + to_string(currentPoints); //currentPoints will be shown
            SDL_Surface *surf = TTF_RenderText_Solid(scoreFont, ptsMsg.c_str(), color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw, th;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                //Will appear below CONGRATULATIONS image
                SDL_Rect dst = {windowWidth / 2 - tw / 2, windowHeight / 2 - 20, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(scoreFont);
        }

        //HOME button
        SDL_Rect homeBtnRect = {windowWidth / 2 - 140, windowHeight / 2 + 40, 110, 55};
        SDL_Rect homeDraw = congratsHomeHovered ? scaleRect(homeBtnRect, 1.11f) : homeBtnRect;
        SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
        SDL_RenderFillRect(renderer, &homeDraw);
        if (congratsHomeHovered)
        {
            SDL_SetRenderDrawColor(renderer, 180, 180, 120, 255);
            SDL_RenderDrawRect(renderer, &homeDraw);
        }
        TTF_Font *btnFont = TTF_OpenFont("../assets/font.ttf", 32);
        if (btnFont)
        {
            SDL_Color btnColor = {0, 0, 0, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(btnFont, "HOME", btnColor);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw, th;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {homeDraw.x + homeDraw.w / 2 - tw / 2, homeDraw.y + homeDraw.h / 2 - th / 2, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(btnFont);
        }

        //NEXT LEVEL button
        SDL_Rect nextBtnRect = {windowWidth / 2 + 5, windowHeight / 2 + 40, 230, 55};
        SDL_Rect nextDraw = congratsNextHovered ? scaleRect(nextBtnRect, 1.11f) : nextBtnRect;
        SDL_SetRenderDrawColor(renderer, 110, 255, 160, 255);
        SDL_RenderFillRect(renderer, &nextDraw);
        if (congratsNextHovered)
        {
            SDL_SetRenderDrawColor(renderer, 50, 200, 100, 255);
            SDL_RenderDrawRect(renderer, &nextDraw);
        }
        btnFont = TTF_OpenFont("../assets/font.ttf", 32);
        if (btnFont)
        {
            SDL_Color btnColor = {0, 0, 0, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(btnFont, "NEXT LEVEL", btnColor);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw, th;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {nextDraw.x + nextDraw.w / 2 - tw / 2, nextDraw.y + nextDraw.h / 2 - th / 2, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(btnFont);
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        return;
    }
}

//Handles all the mouse and keyboard input
void ScreenLevel1::handleEvent(const SDL_Event &e)
{
    int mx = 0, my = 0;
    if (levelComplete)
    {
        //Allow overlay buttons for HOME/NEXT LEVEL
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            mx = e.button.x;
            my = e.button.y;
            SDL_Rect homeBtnRect = {windowWidth / 2 - 140, windowHeight / 2 + 40, 110, 55};
            SDL_Rect nextBtnRect = {windowWidth / 2 + 5, windowHeight / 2 + 40, 230, 55};

            SDL_Point mousePoint = {mx, my};
            if (SDL_PointInRect(&mousePoint, &homeBtnRect))
            {
                game->setLevelCompleted(1);
                game->setHomeShowContinue(true);
                game->setState(GameState::HOME);
                levelComplete = false;
                return;
            }
            if (SDL_PointInRect(&mousePoint, &nextBtnRect))
            {
                game->setLevelCompleted(1);         //Marks progress as completed for level 1
                game->setState(GameState::LEVEL_2); //Loads level 2 //Gamestate is set to LEVEL_2
                return;
            }
        }

        if (e.type == SDL_MOUSEMOTION)
        {
            int mx = e.motion.x, my = e.motion.y;
            SDL_Point mousePoint = {mx, my};
            SDL_Rect homeBtnRect = {windowWidth / 2 - 140, windowHeight / 2 + 40, 110, 55};
            SDL_Rect nextBtnRect = {windowWidth / 2 + 5, windowHeight / 2 + 40, 230, 55};

            congratsHomeHovered = SDL_PointInRect(&mousePoint, &homeBtnRect);
            congratsNextHovered = SDL_PointInRect(&mousePoint, &nextBtnRect);
        }
        //No gameplay allowed while levelComplete, so return
        return;
    }

    if (e.type == SDL_MOUSEMOTION)
    {
        mx = e.motion.x;
        my = e.motion.y;
        SDL_Point mousePoint = {mx, my};
        homeButtonHovered = (gameOver && SDL_PointInRect(&mousePoint, &homeButtonRect));
        retryButtonHovered = (gameOver && SDL_PointInRect(&mousePoint, &retryButtonRect));
        //Pause menu hover
        pauseHomeHovered = (paused && SDL_PointInRect(&mousePoint, &pauseHomeRect));
        pauseContinueHovered = (paused && SDL_PointInRect(&mousePoint, &pauseContinueRect));
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
    {
        mx = e.button.x;
        my = e.button.y;
        SDL_Point mousePoint = {mx, my};

        if (gameOver)
        {
            if (SDL_PointInRect(&mousePoint, &homeButtonRect))
            {
                game->saveProgress(1);
                game->setHomeShowContinue(true);
                game->setState(GameState::HOME);
                return;
            }
            else if (SDL_PointInRect(&mousePoint, &retryButtonRect))
            {
                reset();
                gameOver = false;
                lives = 3;
                return;
            }
        }

        //Pause Menu Buttons
        if (paused)
        {
            if (SDL_PointInRect(&mousePoint, &pauseHomeRect))
            {
                paused = false;
                game->saveProgress(1);
                game->setHomeShowContinue(true);
                game->setState(GameState::HOME);
                return;
            }
            if (SDL_PointInRect(&mousePoint, &pauseContinueRect))
            {
                paused = false; // Unpause and continue game
                return;
            }
        }

        //Pause/play toggle (top right button)
        //Mouse click toggles pause
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            int mx = e.button.x, my = e.button.y;
            if (mx >= buttonRect.x && mx <= buttonRect.x + buttonRect.w &&
                my >= buttonRect.y && my <= buttonRect.y + buttonRect.h)
            {
                paused = !paused;
                return;
            }
        }
    }

    //Key press 'P' toggles pause
    if (e.type == SDL_KEYDOWN && !e.key.repeat && e.key.keysym.sym == SDLK_p)
    {
        paused = !paused;
        return;
    }

    //No gameplay allowed when paused/gameOver
    if (paused || gameOver)
        return;

    //Jumping (Only in Phase 1)
    if (!jumpingPhaseOver && e.type == SDL_KEYDOWN && !e.key.repeat)
    {
        if ((e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_SPACE) && !isJumping)
        {
            isJumping = true;
            velocityY = -20.0f;
        }
    }

    //Ball velocity control
    if (settingVelocities)
    {
        if (e.type == SDL_KEYDOWN && !e.key.repeat) //SDL_KEYDOWN means key is held
        {
            if (e.key.keysym.sym == SDLK_s) //key pressed S
                sHeld = true;
            if (e.key.keysym.sym == SDLK_k) //key pressed K
                kHeld = true;
        }
        else if (e.type == SDL_KEYUP) //SDL_KEYUP means key is released
        {
            if (e.key.keysym.sym == SDLK_s) //key pressed S
                sHeld = false;
            if (e.key.keysym.sym == SDLK_k) //key pressed K
                kHeld = false;
        }
    }
}

//Resets variables to start a new attempt eg. resets player position, obstacles, lives, ball state, UI states everything to initial value
void ScreenLevel1::reset()
{
    currentFrame = 0;
    lastFrameTime = SDL_GetTicks();
    ground1.x = 0;
    ground2.x = groundWidth;
    playerX = 100;
    playerY = groundY;
    isJumping = false;
    velocityY = 0.0f;

    //Obstacle/jumping phase
    obstacleRect.x = windowWidth + rand() % 200;
    obstaclesCleared = 0;
    jumpingPhaseOver = false;
    jumpingPhaseEndTime = 0;

    //Ball phase
    ballPhaseStarted = false;
    ballOnScreen = false;
    footballX = 0;
    footballY = 0;
    ballInMotion = false;
    setVX = setVY = vxBar = vyBar = 0;
    sHeld = kHeld = false;
    settingVelocities = false;

    showGoalCircle = false;
    levelComplete = false;

    showGoalCircle = false;
    levelComplete = false;

    //Game over, UI
    paused = false;
    homeButtonHovered = false;
    retryButtonHovered = false;
    pauseHomeHovered = false;
    pauseContinueHovered = false;
    currentPoints = 0; // lastGainedPoints does not reset here (only on success or new player)
}