//Details of all the modules are already given in ScreenLevel1.cpp

#include "../include/ScreenLevel2.h"
#include "../include/Game.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <random>

using namespace std;

//Helper: scale rect (same as Level 1)
static SDL_Rect scaleRect(const SDL_Rect &rect, float scale)
{
    int newW = static_cast<int>(rect.w * scale);
    int newH = static_cast<int>(rect.h * scale);
    int newX = rect.x - (newW - rect.w) / 2;
    int newY = rect.y - (newH - rect.h) / 2;
    return SDL_Rect{newX, newY, newW, newH};
}

ScreenLevel2::ScreenLevel2(Game *game, SDL_Renderer *renderer, int winWidth, int winHeight)
    : game(game), renderer(renderer),
      backgroundTexture(nullptr), groundTexture(nullptr), playerTexture(nullptr),
      groundWidth(64), groundHeight(0),
      frameWidth(96), frameHeight(96), totalFrames(10), currentFrame(0),
      frameDelay(100), lastFrameTime(SDL_GetTicks()),
      scrollSpeed(27), //Faster for Level 2
      isJumping(false), velocityY(0.0f), gravity(1.5f),
      groundY(0), pauseTexture(nullptr), playTexture(nullptr),
      obstacleTexture1(nullptr), obstacleTexture2(nullptr), obstacleTexture3(nullptr),
      obstacleSpeed(26), playerX(100), playerY(0), windowWidth(winWidth), windowHeight(winHeight),
      homeButtonHovered(false), retryButtonHovered(false), lives(3), heartTexture(nullptr),
      attemptNumber(0), attemptMessageTime(0),
      obstaclesCleared(0), jumpingPhaseOver(false), jumpingPhaseEndTime(0),
      ballPhaseStarted(false), ballOnScreen(false), settingVelocities(false),
      footballTexture(nullptr), footballX(0), footballY(0),
      ballX(0), ballY(0), ballVX(0), ballVY(0), ballInMotion(false),
      sHeld(false), kHeld(false), setVX(0), setVY(0), vxBar(0), vyBar(0), velocityStartTime(0),
      gameOver(false), paused(false),
      pauseHomeHovered(false), pauseContinueHovered(false), playerName(""),
      currentPoints(0), highScore(0), lastGainedPoints(0), lastLifeBonus(0),
      showDoubleRings(false), showGoalCircle(false), levelComplete(false),
      congratsHomeHovered(false), congratsNextHovered(false),
      scoredUpperRing(false), scoredLowerRing(false), doubleRingTexture(nullptr)
{
    std::cout << "[ScreenLevel2] CONSTRUCT at " << this << std::endl;

    rng.seed(SDL_GetTicks()); //for randomness

    // Load resources
    backgroundTexture = IMG_LoadTexture(renderer, "../assets/level1_stadium.png"); 
    if (!backgroundTexture)
        cout << "Failed to load background: " << IMG_GetError() << endl;
    backgroundRect = {0, 0, 1600, 664};

    groundTexture = IMG_LoadTexture(renderer, "../assets/level1_ground.png");
    if (!groundTexture)
        cout << "Failed to load ground: " << IMG_GetError() << endl;
    SDL_QueryTexture(groundTexture, NULL, NULL, &groundWidth, &groundHeight);

    ground1 = {0, windowHeight - groundHeight + 70, groundWidth, groundHeight};
    ground2 = {groundWidth, windowHeight - groundHeight + 70, groundWidth, groundHeight};

    playerY = ground1.y + groundHeight - frameHeight;
    groundY = playerY;

    playerTexture = IMG_LoadTexture(renderer, "../assets/sprite1.png");
    if (!playerTexture)
        cout << "Failed to load player sprite: " << IMG_GetError() << endl;

    playerY = windowHeight - groundHeight - frameHeight + 70;
    groundY = playerY;

    pauseTexture = IMG_LoadTexture(renderer, "../assets/pause.png");
    playTexture = IMG_LoadTexture(renderer, "../assets/play.png");
    buttonRect = {windowWidth - 80, 20, 64, 64};

    //Obstacles - Level 2 uses 3 textures
    obstacleTexture1 = IMG_LoadTexture(renderer, "../assets/obstacle.png");  //Rectangle (Cone)
    obstacleTexture2 = IMG_LoadTexture(renderer, "../assets/obstacle2.png"); //Rectangle 1(3 Cones)
    obstacleTexture3 = IMG_LoadTexture(renderer, "../assets/obstacle3.png"); //Rectangle 2 (Drums)

    //Initial obstacle layout: 10 obstacles, random order
    obstacleSequence.clear();
    for (int i = 0; i < maxObstacles; ++i)
    {
        obstacleSequence.push_back((i % 3) + 1); //at least one of each, results in a sequence of 1, 2, 3
    }
    shuffle(obstacleSequence.begin(), obstacleSequence.end(), rng); //shuffles the order of obstacles and generates a random one next(rng)

    obstacles.clear();
    for (int i = 0; i < maxObstacles; ++i)
    {
        Obstacle obs;
        obs.type = obstacleSequence[i];
        obs.rect = {windowWidth + 200 + i * 300, ground1.y - 50 + 20, 60, 100}; //Will size by type later
        obstacles.push_back(obs);
    }

    // Ball
    footballTexture = IMG_LoadTexture(renderer, "../assets/football.png");
    if (!footballTexture)
        cout << "Failed to load football: " << IMG_GetError() << endl;

    // Heart
    heartTexture = IMG_LoadTexture(renderer, "../assets/heart.png");
    if (!heartTexture)
        cout << "Failed to load heart: " << IMG_GetError() << endl;

        // Load the double ring PNG
    doubleRingTexture = IMG_LoadTexture(renderer, "../assets/goal_circle2.png");
    if (!doubleRingTexture)
        std::cout << "[ERROR] Could not load doubleRingTexture from ../assets/goal_circle2.png: " << IMG_GetError() << std::endl;
    else
        std::cout << "[OK] Loaded doubleRingTexture, ptr: " << doubleRingTexture << std::endl;

    // Pause menu rects (center popup)
    pauseMenuRect = {winWidth / 2 - 200, winHeight / 2 - 130, 450, 260};
    pauseHomeRect = {winWidth / 2 - 150, winHeight / 2 + 20, 130, 60};
    pauseContinueRect = {winWidth / 2 + 30, winHeight / 2 + 20, 200, 60};

    homeButtonRect = {winWidth / 2 - 120, winHeight / 2 + 40, 100, 50};
    retryButtonRect = {winWidth / 2 + 20, winHeight / 2 + 40, 150, 50};

    reset(true);

}

ScreenLevel2::~ScreenLevel2()
{


    std::cout << "[ScreenLevel2] destructor called at " << this << ", doubleRingTexture: " << doubleRingTexture << std::endl;
    //debug code above


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
    if (obstacleTexture1)
        SDL_DestroyTexture(obstacleTexture1);
    if (obstacleTexture2)
        SDL_DestroyTexture(obstacleTexture2);
    if (obstacleTexture3)
        SDL_DestroyTexture(obstacleTexture3);
    if (heartTexture)
        SDL_DestroyTexture(heartTexture);
    if (footballTexture)
        SDL_DestroyTexture(footballTexture);
    if (doubleRingTexture){
        SDL_DestroyTexture(doubleRingTexture);
        doubleRingTexture = nullptr;
    }

}

void ScreenLevel2::reset(bool fullReset)
{
    currentFrame = 0;
    lastFrameTime = SDL_GetTicks();
    ground1.x = 0;
    ground2.x = groundWidth;
    playerX = 100;
    playerY = groundY;
    isJumping = false;
    velocityY = 0.0f;

    if (fullReset) {
        lives = 3;
        attemptNumber = 1;
    }

    // Obstacles
    obstaclesCleared = 0;
    jumpingPhaseOver = false;
    jumpingPhaseEndTime = 0;
    obstacles.clear();
    shuffle(obstacleSequence.begin(), obstacleSequence.end(), rng);
    int baseX = windowWidth + 200;
    int spacing = 320;

    for (int i = 0; i < maxObstacles; ++i)
    {
        Obstacle obs;
        obs.type = obstacleSequence[i];
        int obsW, obsH;
        if (obs.type == 1)      { obsW = 60;  obsH = 160; }
        else if (obs.type == 2) { obsW = 140; obsH = 140; }
        else                    { obsW = 160; obsH = 140; }
        int obsY = ground1.y + groundHeight - obsH-180; // Sits on ground!
        int obsX = windowWidth + 200 + i * 320;
        obs.rect = {obsX, obsY, obsW, obsH};
        obstacles.push_back(obs);
    }

    // Ball phase
    ballPhaseStarted = false;
    ballOnScreen = false;
    footballX = 0;
    footballY = 0;
    ballInMotion = false;
    setVX = setVY = vxBar = vyBar = 0;
    sHeld = kHeld = false;
    settingVelocities = false;

    showDoubleRings = false;
    showGoalCircle = false;
    levelComplete = false;
    scoredUpperRing = false;
    scoredLowerRing = false;

    // Game over, UI
    paused = false;
    homeButtonHovered = false;
    retryButtonHovered = false;
    pauseHomeHovered = false;
    pauseContinueHovered = false;
    currentPoints = 0;
}

void ScreenLevel2::update()
{
    if (levelComplete || paused || gameOver)
        return;

    // Jumping physics
    if (isJumping)
    {
        playerY += static_cast<int>(velocityY);
        velocityY += gravity;
        if (playerY >= groundY)
        {
            playerY = groundY;
            isJumping = false;
            velocityY = 0.0f;
        }
    }

    // Animate player
    if (SDL_GetTicks() - lastFrameTime >= frameDelay)
    {
        currentFrame = (currentFrame + 1) % totalFrames;
        lastFrameTime = SDL_GetTicks();
    }

    // Scroll ground
    ground1.x -= scrollSpeed;
    ground2.x -= scrollSpeed;
    if (ground1.x + groundWidth <= 0)
        ground1.x = ground2.x + groundWidth;
    if (ground2.x + groundWidth <= 0)
        ground2.x = ground1.x + groundWidth;

    if (showDoubleRings && !ballInMotion) {
        doubleRingRect.x -= scrollSpeed;
        lowerRingRect.x  -= scrollSpeed;
        upperRingRect.x  -= scrollSpeed;
    }

    //PHASE 1: 10 OBSTACLE JUMPS
    if (!jumpingPhaseOver)
    {
        if (obstaclesCleared < maxObstacles)
        {
            Obstacle &obs = obstacles[obstaclesCleared];

            // Move obstacle
            obs.rect.x -= obstacleSpeed;

            if (obs.rect.x + obs.rect.w < 0)
            {
                obstaclesCleared++;
                currentPoints += 25; // Each jump: +30

                if (obstaclesCleared < maxObstacles)
                {
                    int type = obstacleSequence[obstaclesCleared];
                    int obsW, obsH;
                    if (type == 1)      { obsW = 60;  obsH = 160; }
                    else if (type == 2) { obsW = 140; obsH = 140; }
                    else                { obsW = 160; obsH = 140; }
                    int obsY = ground1.y + groundHeight - obsH-180;
                    int obsX = windowWidth + 200;
                    obstacles[obstaclesCleared].rect = {obsX, obsY, obsW, obsH};
                }

                else
                {
                    jumpingPhaseOver = true;
                    jumpingPhaseEndTime = SDL_GetTicks();
                    ballPhaseStarted = false;
                    ballOnScreen = false;
                    settingVelocities = true;
                    velocityStartTime = SDL_GetTicks();
                    vxBar = vyBar = 0;
                    setVX = setVY = 0;
                    sHeld = kHeld = false;
                    playerX = 100;
                    playerY = groundY;
                }
            }

            // Player collision
            SDL_Rect playerRect = {playerX, playerY - 15, frameWidth + 40, frameHeight + 100};
            if (SDL_HasIntersection(&playerRect, &obs.rect))
            {
                if (lives > 1)
                {
                    lives--;
                    attemptNumber = 4 - lives;
                    attemptMessageTime = SDL_GetTicks();
                    playerY = groundY;
                    isJumping = false;
                    velocityY = 0.0f;
                    currentPoints = 0;    // Reset points on fail
                    lastGainedPoints = 0; // Reset last gained as well
                    reset(false); // Restart level 2
                }
                else
                {
                    lives = 0;
                    gameOver = true;
                    currentPoints = 0;    // Reset points on fail
                    lastGainedPoints = 0; // Reset last gained as well
                }
                return;
            }
            return;
        }
    }

    //PHASE 2: VELOCITY BAR
    Uint32 timeSinceJumpingOver = SDL_GetTicks() - jumpingPhaseEndTime;
    if (!ballPhaseStarted && timeSinceJumpingOver <= 2700)
    {
        if (settingVelocities)
        {
            if (sHeld)
                vyBar = min(1.0f, vyBar + 0.031f); // Slightly faster fill as scrolling is fast in this level
            if (kHeld)
                vxBar = min(1.0f, vxBar + 0.031f);
        }
        playerX -= scrollSpeed;
        if (playerX < 100)
            playerX = 100;
        return;
    }

    //PHASE 3: BALL & DOUBLE RINGS
    if (!ballPhaseStarted && timeSinceJumpingOver > 2700)
    {
        ballPhaseStarted = true;
        ballOnScreen = true;
        settingVelocities = false;

        footballX = windowWidth - 100;
        footballY = groundY + frameHeight;
        ballX = footballX;
        ballY = footballY;

        int ringGap = 700; // how far in front of the ball to place the double rings
        int baseX = (int)ballX + ringGap;
        int baseY = (int)ballY - 170;

        doubleRingRect = {baseX, baseY - 120, 300, 400};
        lowerRingRect = {baseX + 100, baseY + 93, 75, 87};
        upperRingRect = {baseX + 110, baseY - 55, 60, 68};


        showDoubleRings = true;

    }

    if (!ballPhaseStarted)
        return;

    if (ballOnScreen && !ballInMotion)
        ballX -= scrollSpeed;

    // Player auto-runs to ball
    if (ballOnScreen && !ballInMotion)
    {
        if (playerX + frameWidth + 30 < ballX)
            playerX += max(2, scrollSpeed / 4);
        else
        {
            if (vxBar == 0 && vyBar == 0)
            {
                if (lives > 1)
                {
                    lives--;
                    attemptNumber = 4 - lives;
                    attemptMessageTime = SDL_GetTicks();
                    reset(false);
                    return;
                }
                else
                {
                    lives = 0;
                    gameOver = true;
                    return;
                }
            }
            // Kick!
            ballInMotion = true;
            setVX = vxBar;
            setVY = vyBar;
            ballVX = max(8.0f, vxBar * 47.0f);
            ballVY = max(-22.0f, -vyBar * 52.0f);
        }
    }

    // Ball motion: projectile
    if (ballInMotion)
    {
        ballX += ballVX;
        ballY += ballVY;
        ballVY += 0.67f; // gravity

        SDL_Rect ballRect = {(int)ballX, (int)ballY, 42, 42};
        // Score for upper ring (first)
        if (!scoredUpperRing && SDL_HasIntersection(&ballRect, &upperRingRect))
        {
            scoredUpperRing = true;
            currentPoints += 200;
        }
        // Score for lower ring (if not upper)
        else if (!scoredLowerRing && SDL_HasIntersection(&ballRect, &lowerRingRect))
        {
            scoredLowerRing = true;
            currentPoints += 100;
        }

        // Level complete if scored either ring
        if (scoredUpperRing || scoredLowerRing)
        {
            showDoubleRings = false;
            ballInMotion = false;
            levelComplete = true;
            int bonus = lives * 50;
            int gained = (scoredUpperRing ? 200 : 100) + bonus;
            lastGainedPoints = gained;
            lastLifeBonus = bonus;
            currentPoints += bonus;

            // High score logic
            if (currentPoints > highScore)
            {
                highScore = currentPoints;
                game->setScoreForPlayer(playerName, highScore, 2);
            }
            game->setLevelCompleted(2);
            return;
        }

        // FAIL: ball missed both rings (out of bounds or ground)
        int groundLevel = footballY;
        if (ballY >= groundLevel + 80 || ballX > windowWidth + 300 || ballX < -64)
        {
            ballInMotion = false;
            currentPoints = 0;    // Reset points on fail
            lastGainedPoints = 0; // Reset last gained as well
            if (lives > 1)
            {
                lives--;
                attemptNumber = 4 - lives;
                attemptMessageTime = SDL_GetTicks();
                reset(false);
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

void ScreenLevel2::render()
{
    // Draw background
    if (backgroundTexture)
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &backgroundRect);

    // Draw ground
    if (groundTexture)
    {
        SDL_RenderCopy(renderer, groundTexture, NULL, &ground1);
        SDL_RenderCopy(renderer, groundTexture, NULL, &ground2);
    }

    // Draw double rings (rectangles for now)
    if (showDoubleRings && doubleRingTexture)
    {
        // Draw PNG
        SDL_RenderCopy(renderer, doubleRingTexture, NULL, &doubleRingRect);

        /* Draw debug: show upper/lower ring hitboxes as outlines
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 160); // green for upper
        SDL_RenderDrawRect(renderer, &upperRingRect);
        SDL_SetRenderDrawColor(renderer, 255, 140, 0, 160); // orange for lower
        SDL_RenderDrawRect(renderer, &lowerRingRect);
        */

        // Draw points label for each ring
        TTF_Font *ringFont = TTF_OpenFont("../assets/font.ttf", 24);
        if (ringFont)
        {
            SDL_Color color = {255, 255, 255, 255};
            SDL_Surface *surf1 = TTF_RenderText_Solid(ringFont, "200", color);
            SDL_Surface *surf2 = TTF_RenderText_Solid(ringFont, "100", color);
            if (surf1)
            {
                SDL_Texture *txt1 = SDL_CreateTextureFromSurface(renderer, surf1);
                SDL_Rect dst1 = {upperRingRect.x + upperRingRect.w / 2 - surf1->w / 2, upperRingRect.y - 26, surf1->w, surf1->h};
                SDL_RenderCopy(renderer, txt1, NULL, &dst1);
                SDL_FreeSurface(surf1);
                SDL_DestroyTexture(txt1);
            }
            if (surf2)
            {
                SDL_Texture *txt2 = SDL_CreateTextureFromSurface(renderer, surf2);
                SDL_Rect dst2 = {lowerRingRect.x + lowerRingRect.w / 2 - surf2->w / 2, lowerRingRect.y + lowerRingRect.h + 6, surf2->w, surf2->h};
                SDL_RenderCopy(renderer, txt2, NULL, &dst2);
                SDL_FreeSurface(surf2);
                SDL_DestroyTexture(txt2);
            }
            TTF_CloseFont(ringFont);
        }
    }


    // Draw hearts
    if (heartTexture)
    {
        int heartW = 40, heartH = 40, spacing = 8;
        for (int i = 0; i < lives; ++i)
        {
            SDL_Rect dst = {10 + i * (heartW + spacing), 10, heartW, heartH};
            SDL_RenderCopy(renderer, heartTexture, NULL, &dst);
        }
    }

    // Show current points
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

    // Player name at top center
    if (!playerName.empty())
    {
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 36);
        if (font)
        {
            SDL_Color color = {255, 215, 0, 255};
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

    // Draw attempt message if appropriate
    if (attemptNumber >= 2 && SDL_GetTicks() - attemptMessageTime < 1200)
    {
        string msg = "ATTEMPT " + to_string(attemptNumber);
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 56);
        if (font)
        {
            SDL_Color color = {255, 215, 0, 255};
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

    // Draw obstacles (phase 1 only)
    if (!jumpingPhaseOver && !gameOver)
    {
        if (!jumpingPhaseOver && !gameOver)
        {
            for (int i = obstaclesCleared; i < maxObstacles; ++i) {
                Obstacle& obs = obstacles[i];
                SDL_Texture* tex = (obs.type == 1 ? obstacleTexture1 : obs.type == 2 ? obstacleTexture2 : obstacleTexture3);
                if (tex)
                    SDL_RenderCopy(renderer, tex, NULL, &obs.rect);
            }
        }

        // How many left
        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 36);
        if (font)
        {
            string counter = to_string(max(0, maxObstacles - obstaclesCleared)) + " left";
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

    // Draw player
    if (playerTexture && !gameOver)
    {
        SDL_Rect srcRect = {currentFrame * frameWidth, 0, frameWidth, frameHeight};
        SDL_Rect destRect = {playerX, playerY - 15, frameWidth + 40, frameHeight + 100};
        SDL_RenderCopy(renderer, playerTexture, &srcRect, &destRect);
    }

    // Draw football
    if (ballOnScreen || ballInMotion)
    {
        if (footballTexture)
        {
            SDL_Rect ballDst = {(int)ballX, (int)ballY, 42, 42};
            SDL_RenderCopy(renderer, footballTexture, NULL, &ballDst);
        }
    }

    // VELOCITY BARS
    Uint32 timeSinceJumpingOver = SDL_GetTicks() - jumpingPhaseEndTime;
    if (jumpingPhaseOver && !ballPhaseStarted && timeSinceJumpingOver <= 2700)
    {
        SDL_Rect vxBarBG = {windowWidth / 2 - 200, windowHeight - 70, 180, 24};
        SDL_Rect vyBarBG = {windowWidth / 2 + 20, windowHeight - 70, 180, 24};
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_RenderFillRect(renderer, &vxBarBG);
        SDL_RenderFillRect(renderer, &vyBarBG);

        SDL_SetRenderDrawColor(renderer, 20, 200, 50, 255);
        SDL_Rect vxFill = vxBarBG;
        vxFill.w = (int)(vxBar * vxBarBG.w);
        SDL_Rect vyFill = vyBarBG;
        vyFill.w = (int)(vyBar * vyBarBG.w);
        SDL_RenderFillRect(renderer, &vxFill);
        SDL_RenderFillRect(renderer, &vyFill);

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

    // Draw pause/play button
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

    // PAUSE MENU OVERLAY 
    if (paused)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 180);
        SDL_RenderFillRect(renderer, &pauseMenuRect);

        // PAUSED text
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
        // HOME and CONTINUE buttons
        SDL_Rect homeDraw = pauseHomeHovered ? scaleRect(pauseHomeRect, 1.11f) : pauseHomeRect;
        SDL_SetRenderDrawColor(renderer, 245, 210, 40, 255);
        SDL_RenderFillRect(renderer, &homeDraw);
        if (pauseHomeHovered)
        {
            SDL_SetRenderDrawColor(renderer, 180, 140, 40, 255);
            SDL_RenderDrawRect(renderer, &homeDraw);
        }
        SDL_Rect contDraw = pauseContinueHovered ? scaleRect(pauseContinueRect, 1.11f) : pauseContinueRect;
        SDL_SetRenderDrawColor(renderer, 60, 220, 140, 255);
        SDL_RenderFillRect(renderer, &contDraw);
        if (pauseContinueHovered)
        {
            SDL_SetRenderDrawColor(renderer, 30, 140, 80, 255);
            SDL_RenderDrawRect(renderer, &contDraw);
        }
        TTF_Font *btnFont = TTF_OpenFont("../assets/font.ttf", 32);
        if (btnFont)
        {
            SDL_Color btnColor = {30, 30, 30, 255};
            // HOME label
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
            // CONTINUE label
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
        return;
    }

    // GAME OVER overlay
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
            // Show Points/High Score
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
                    SDL_Rect dst = {windowWidth / 2 - tw / 2 + 32, windowHeight / 2 - 20, tw, th};
                    SDL_RenderCopy(renderer, txt, NULL, &dst);
                    SDL_FreeSurface(surf);
                    SDL_DestroyTexture(txt);
                }
                TTF_CloseFont(pointsFont);
            }
        }

        // POP-UP BUTTONS
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

            // RETRY button
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
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 180);
        SDL_Rect overlayRect = {0, 0, windowWidth, windowHeight};
        SDL_RenderFillRect(renderer, &overlayRect);

        TTF_Font *font = TTF_OpenFont("../assets/font.ttf", 52);
        if (font)
        {
            SDL_Color color = {255, 215, 0, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(font, "LEVEL 2 COMPLETE!", color);
            if (surf)
            {
                SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                int tw, th;
                SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                SDL_Rect dst = {windowWidth / 2 - tw / 2, windowHeight / 2 - 130, tw, th};
                SDL_RenderCopy(renderer, txt, NULL, &dst);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(txt);
            }
            TTF_CloseFont(font);

            // Points/bonus
            TTF_Font *pointsFont = TTF_OpenFont("../assets/font.ttf", 40);
            if (pointsFont)
            {
                SDL_Color color = {255, 255, 255, 255};
                string pointsMsg = "Points: " + to_string(currentPoints);
                SDL_Surface *surf = TTF_RenderText_Solid(pointsFont, pointsMsg.c_str(), color);
                if (surf)
                {
                    SDL_Texture *txt = SDL_CreateTextureFromSurface(renderer, surf);
                    int tw, th;
                    SDL_QueryTexture(txt, NULL, NULL, &tw, &th);
                    SDL_Rect dst = {windowWidth / 2 - tw / 2, windowHeight / 2 - 38, tw, th};
                    SDL_RenderCopy(renderer, txt, NULL, &dst);
                    SDL_FreeSurface(surf);
                    SDL_DestroyTexture(txt);
                }
                TTF_CloseFont(pointsFont);
            }

            // HOME button
            SDL_Rect homeBtnRect = {windowWidth / 2 - 200, windowHeight / 2 + 40, 110, 55};
            SDL_Rect homeDraw = congratsHomeHovered ? scaleRect(homeBtnRect, 1.11f) : homeBtnRect;
            SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
            SDL_RenderFillRect(renderer, &homeDraw);
            if (congratsHomeHovered) {
                SDL_SetRenderDrawColor(renderer, 180, 180, 120, 255);
                SDL_RenderDrawRect(renderer, &homeDraw);
            }
            TTF_Font *btnFont = TTF_OpenFont("../assets/font.ttf", 32);
            if (btnFont) {
                SDL_Color btnColor = {0, 0, 0, 255};
                SDL_Surface *surf = TTF_RenderText_Solid(btnFont, "HOME", btnColor);
                if (surf) {
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

            // NEXT LEVEL button
            SDL_Rect nextBtnRect = {windowWidth / 2 + 5, windowHeight / 2 + 40, 230, 55};
            SDL_Rect nextDraw = congratsNextHovered ? scaleRect(nextBtnRect, 1.11f) : nextBtnRect;
            SDL_SetRenderDrawColor(renderer, 110, 255, 160, 255);
            SDL_RenderFillRect(renderer, &nextDraw);
            if (congratsNextHovered) {
                SDL_SetRenderDrawColor(renderer, 50, 200, 100, 255);
                SDL_RenderDrawRect(renderer, &nextDraw);
            }
            btnFont = TTF_OpenFont("../assets/font.ttf", 32);
            if (btnFont) {
                SDL_Color btnColor = {0, 0, 0, 255};
                SDL_Surface *surf = TTF_RenderText_Solid(btnFont, "NEXT LEVEL", btnColor);
                if (surf) {
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

        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);


    }
}

void ScreenLevel2::handleEvent(const SDL_Event &e)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    // Pause button click
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
    {
        SDL_Point pt = {mx, my};
        if (SDL_PointInRect(&pt, &buttonRect))
        {
            paused = !paused;
            return;
        }
    }

    // PAUSED: menu buttons
    if (paused)
    {
        SDL_Point pt = {mx, my};
        pauseHomeHovered = SDL_PointInRect(&pt, &pauseHomeRect);
        pauseContinueHovered = SDL_PointInRect(&pt, &pauseContinueRect);
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            if (pauseHomeHovered)
            {
                game->saveProgress(2);
                game->setHomeShowContinue(true);
                game->setState(GameState::HOME);
            }
            else if (pauseContinueHovered)
                paused = false;
        }
        return;
    }

    // GAME OVER: buttons
    if (gameOver)
    {
        SDL_Point pt = {mx, my};
        homeButtonHovered = SDL_PointInRect(&pt, &homeButtonRect);
        retryButtonHovered = SDL_PointInRect(&pt, &retryButtonRect);
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            if (homeButtonHovered)
            {
                game->saveProgress(2); // Save progress (level 2)
                game->setHomeShowContinue(true);
                game->setState(GameState::HOME);
            }
            else if (retryButtonHovered)
            {
                reset(true);
                gameOver = false;
            }
        }
        return;
    }

    if (levelComplete)
    {
        // Allow overlay buttons for HOME/NEXT LEVEL
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            int mx = e.button.x;
            int my = e.button.y;
            SDL_Rect homeBtnRect = {windowWidth / 2 - 200, windowHeight / 2 + 40, 110, 55};
            SDL_Rect nextBtnRect = {windowWidth / 2 + 5, windowHeight / 2 + 40, 230, 55};

            SDL_Point mousePoint = {mx, my};
            if (SDL_PointInRect(&mousePoint, &homeBtnRect))
            {
                game->setLevelCompleted(2); 
                game->setHomeShowContinue(true); 
                game->setState(GameState::HOME);
                levelComplete = false;
                return;
            }
            if (SDL_PointInRect(&mousePoint, &nextBtnRect))
            {
                game->setLevelCompleted(2);         // Marks progress as completed for level 2
                game->setState(GameState::LEVEL_3); //Loads LEVEL_3
                return;
            }
        }

        if (e.type == SDL_MOUSEMOTION)
        {
            int mx = e.motion.x, my = e.motion.y;
            SDL_Point mousePoint = {mx, my};
            SDL_Rect homeBtnRect = {windowWidth / 2 - 200, windowHeight / 2 + 40, 110, 55};
            SDL_Rect nextBtnRect = {windowWidth / 2 + 5, windowHeight / 2 + 40, 230, 55};

            congratsHomeHovered = SDL_PointInRect(&mousePoint, &homeBtnRect);
            congratsNextHovered = SDL_PointInRect(&mousePoint, &nextBtnRect);
        }
        // No gameplay allowed while levelComplete
        return;
    }


    // JUMPING: jump on space
    if (!jumpingPhaseOver && e.type == SDL_KEYDOWN &&
        (e.key.keysym.sym == SDLK_SPACE || e.key.keysym.sym == SDLK_UP))
    {
        if (!isJumping && playerY >= groundY)
        {
            isJumping = true;
            velocityY = -23.0f; // big jump for obstacles
        }
    }

    // VELOCITY BARS: 's' and 'k' keys
    if (jumpingPhaseOver && !ballPhaseStarted)
    {
        if (e.type == SDL_KEYDOWN)
        {
            if (e.key.keysym.sym == SDLK_s)
                sHeld = true;
            if (e.key.keysym.sym == SDLK_k)
                kHeld = true;
        }
        if (e.type == SDL_KEYUP)
        {
            if (e.key.keysym.sym == SDLK_s)
                sHeld = false;
            if (e.key.keysym.sym == SDLK_k)
                kHeld = false;
        }
    }
}
