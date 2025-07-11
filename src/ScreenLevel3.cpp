//Details of all the modules are already given in ScreenLevel1.cpp

#include "../include/ScreenLevel3.h"
#include "../include/Game.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <random>

using namespace std;

// Helper: scale rect (same as Level 1)
static SDL_Rect scaleRect(const SDL_Rect &rect, float scale)
{
    int newW = static_cast<int>(rect.w * scale);
    int newH = static_cast<int>(rect.h * scale);
    int newX = rect.x - (newW - rect.w) / 2;
    int newY = rect.y - (newH - rect.h) / 2;
    return SDL_Rect{newX, newY, newW, newH};
}

ScreenLevel3::ScreenLevel3(Game *game, SDL_Renderer *renderer, int winWidth, int winHeight)
    : game(game), renderer(renderer),
      backgroundTexture(nullptr), groundTexture(nullptr), playerTexture(nullptr),
      groundWidth(64), groundHeight(0),
      frameWidth(96), frameHeight(96), totalFrames(10), currentFrame(0),
      frameDelay(100), lastFrameTime(SDL_GetTicks()),
      scrollSpeed(30), // Faster for Level 3
      isJumping(false), velocityY(0.0f), gravity(1.5f),
      groundY(0), pauseTexture(nullptr), playTexture(nullptr),
      obstacleTexture1(nullptr), obstacleTexture2(nullptr), obstacleTexture3(nullptr),
      obstacleTexture4(nullptr), obstacleTexture5(nullptr),
      obstacleSpeed(30), playerX(100), playerY(0), windowWidth(winWidth), windowHeight(winHeight),
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
      showGoalCircle(false), levelComplete(false),
      congratsHomeHovered(false), congratsNextHovered(false),
      scoredTopHole(false), scoredMidHole(false), scoredBottomHole(false),
      goalCircle3Texture(nullptr)
{
    cout << "[ScreenLevel3] CONSTRUCT at " << this << endl;

    rng.seed(SDL_GetTicks());

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

    // Obstacles - Level 3 uses 5 textures
    obstacleTexture1 = IMG_LoadTexture(renderer, "../assets/obstacle.png");  // Same as Level 1 & 2
    obstacleTexture2 = IMG_LoadTexture(renderer, "../assets/obstacle2.png"); // Same as Level 2
    obstacleTexture3 = IMG_LoadTexture(renderer, "../assets/obstacle3.png"); // Same as Level 2
    obstacleTexture4 = IMG_LoadTexture(renderer, "../assets/obstacle4.png"); // Mannequin
    obstacleTexture5 = IMG_LoadTexture(renderer, "../assets/obstacle5.png"); // Obstacle

    if (!obstacleTexture1)
        cout << "Failed to load obstacle 1: " << IMG_GetError() << endl;
    if (!obstacleTexture2)
        cout << "Failed to load obstacle 2: " << IMG_GetError() << endl;
    if (!obstacleTexture3)
        cout << "Failed to load obstacle 3: " << IMG_GetError() << endl;
    if (!obstacleTexture4)
        cout << "Failed to load obstacle 4: " << IMG_GetError() << endl;
    if (!obstacleTexture5)
        cout << "Failed to load obstacle 5: " << IMG_GetError() << endl;

    // Initial obstacle layout: 15 obstacles, random order but repeating all types equally
    const int maxObstacles = 15;
    obstacleSequence.clear();
    for (int i = 0; i < maxObstacles; ++i)
    {
        obstacleSequence.push_back((i % 5) + 1); // 1..5, repeat 3 times like level 2 but this time 5 obstacles
    }
    shuffle(obstacleSequence.begin(), obstacleSequence.end(), rng); // shuffles the order of obstacles and generates a random one next(rng)

    obstacles.clear();
    for (int i = 0; i < maxObstacles; ++i)
    {
        Obstacle obs;
        obs.type = obstacleSequence[i];
        obs.rect = {windowWidth + 200 + i * 240, ground1.y - 50 + 20, 128, 128};
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

    // Load triple ring PNG
    goalCircle3Texture = IMG_LoadTexture(renderer, "../assets/goal_circle3.png");
    if (!goalCircle3Texture)
        cout << "Failed to load goal_circle3.png: " << IMG_GetError() << endl;
    else
        cout << "[OK] Loaded goalCircle3Texture, ptr: " << goalCircle3Texture << endl;

    // Pause menu rects (center popup)
    pauseMenuRect = {winWidth / 2 - 200, winHeight / 2 - 130, 450, 260};
    pauseHomeRect = {winWidth / 2 - 150, winHeight / 2 + 20, 130, 60};
    pauseContinueRect = {winWidth / 2 + 30, winHeight / 2 + 20, 200, 60};

    homeButtonRect = {winWidth / 2 - 120, winHeight / 2 + 40, 100, 50};
    retryButtonRect = {winWidth / 2 + 20, winHeight / 2 + 40, 150, 50};

    // The ring system: triple ring hitboxes
    goalCircle3Rect = {windowWidth - 200, groundY - 50, 300, 500};
    topHoleRect = {goalCircle3Rect.x + 120, goalCircle3Rect.y + 70, 62, 72};
    midHoleRect = {goalCircle3Rect.x + 110, goalCircle3Rect.y + 205, 80, 83};

    bottomHoleRect = {goalCircle3Rect.x + 108, goalCircle3Rect.y + 355, 85, 88};

    reset(true);
}

ScreenLevel3::~ScreenLevel3()
{
    cout << "[ScreenLevel3] destructor called at " << this << ", goalCircle3Texture: " << goalCircle3Texture << endl;

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

    // Destroy all obstacle textures
    if (obstacleTexture1)
        SDL_DestroyTexture(obstacleTexture1);
    if (obstacleTexture2)
        SDL_DestroyTexture(obstacleTexture2);
    if (obstacleTexture3)
        SDL_DestroyTexture(obstacleTexture3);
    if (obstacleTexture4)
        SDL_DestroyTexture(obstacleTexture4);
    if (obstacleTexture5)
        SDL_DestroyTexture(obstacleTexture5);

    if (heartTexture)
        SDL_DestroyTexture(heartTexture);
    if (footballTexture)
        SDL_DestroyTexture(footballTexture);

    // Triple ring goal
    if (goalCircle3Texture)
    {
        SDL_DestroyTexture(goalCircle3Texture);
        goalCircle3Texture = nullptr;
    }
}

void ScreenLevel3::reset(bool fullReset)
{
    currentFrame = 0;
    lastFrameTime = SDL_GetTicks();
    ground1.x = 0;
    ground2.x = groundWidth;
    playerX = 100;
    playerY = groundY;
    isJumping = false;
    velocityY = 0.0f;

    if (fullReset)
    {
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
    int spacing = 240; // Tighter spacing for 15 obstacles

    for (int i = 0; i < maxObstacles; ++i)
    {
        Obstacle obs;
        obs.type = obstacleSequence[i];

        int obsW = 128, obsH = 128;
        // Assign size by type
        switch (obs.type)
        {
        case 1: // 1 cone
            obsW = 135;
            obsH = 165;
            break;
        case 2:   // 3 cones
            obsW = 190;
            obsH = 150;
            break;
        case 3: // drum
            obsW = 270;
            obsH = 130;
            break;
        case 4: // Mannequin
            obsW = 150;
            obsH = 167;
            break;
        case 5: // goalbox
            obsW = 180;
            obsH = 160;
            break;
        default:
            obsW = 128;
            obsH = 128;
            break;
        }

        int obsY = ground1.y + groundHeight - obsH - 180;
        int obsX = baseX + i * spacing;
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

    // Triple ring/goal system
    showGoalCircle = false;
    levelComplete = false;
    scoredTopHole = false;
    scoredMidHole = false;
    scoredBottomHole = false;

    // Game over, UI
    paused = false;
    homeButtonHovered = false;
    retryButtonHovered = false;
    pauseHomeHovered = false;
    pauseContinueHovered = false;
    currentPoints = 0;

    // Centered static initial placement
    int goalWidth = 300;
    int goalHeight = 500;
    int goalX = windowWidth / 2 - goalWidth / 2;
    int groundBottom = ground1.y + ground1.h;
    int goalY = groundBottom - goalHeight;

    goalCircle3Rect = {goalX, goalY, goalWidth, goalHeight};
    topHoleRect = {goalX + 120, goalY + 70, 62, 72};
    midHoleRect = {goalX + 110, goalY + 205, 80, 83};
    bottomHoleRect = {goalX + 108, goalY + 355, 85, 88};
}

void ScreenLevel3::update()
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

    // PHASE 1: 15 OBSTACLE JUMPS
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
                currentPoints += 25; // Each jump: +25

                if (obstaclesCleared < maxObstacles)
                {
                    int type = obstacleSequence[obstaclesCleared];

                    int obsW = 128, obsH = 128;
                    switch (type)
                    {
                    case 1:
                        obsW = 135;
                        obsH = 165;
                        break;
                    case 2:
                        obsW = 190;
                        obsH = 150;
                        break;
                    case 3:
                        obsW = 270;
                        obsH = 130;
                        break;
                    case 4:
                        obsW = 150;
                        obsH = 167;
                        break;
                    case 5:
                        obsW = 180;
                        obsH = 160;
                        break;
                    default:
                        obsW = 128;
                        obsH = 128;
                        break;
                    }

                    int obsY = ground1.y + groundHeight - obsH - 180;
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
                    reset(false);         // Restart level 3
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

    // PHASE 2: VELOCITY BAR
    Uint32 timeSinceJumpingOver = SDL_GetTicks() - jumpingPhaseEndTime;
    if (!ballPhaseStarted && timeSinceJumpingOver <= 2700)
    {
        if (settingVelocities)
        {
            if (sHeld)
                vyBar = min(1.0f, vyBar + 0.031f);
            if (kHeld)
                vxBar = min(1.0f, vxBar + 0.031f);
        }
        playerX -= scrollSpeed;
        if (playerX < 100)
            playerX = 100;
        return;
    }

    // PHASE 3: BALL & GOAL CIRCLE
    if (!ballPhaseStarted && timeSinceJumpingOver > 2700)
    {
        ballPhaseStarted = true;
        ballOnScreen = true;
        settingVelocities = false;

        // Goal is static on the right side
        int goalWidth = 450, goalHeight = 550;
        int goalX = windowWidth - goalWidth - 50;
        int groundBottom = ground1.y + ground1.h;
        int goalY = groundBottom - goalHeight - 150;

        goalCircle3Rect = {goalX, goalY, goalWidth, goalHeight};
        topHoleRect = {goalX + 190, goalY + 80, 50, 63};
        midHoleRect = {goalX + 186, goalY + 217, 63, 65};
        bottomHoleRect = {goalX + 180, goalY + 340, 63, 78};

        // The goal is in front(ahead) of the ball
        int ballGap = 350;
        ballX = goalX - ballGap;
        ballY = goalY + goalHeight - 42;
        footballX = ballX;
        footballY = ballY;

        showGoalCircle = true;
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

        // Score: Top hole first
        if (!scoredTopHole && SDL_HasIntersection(&ballRect, &topHoleRect))
        {
            scoredTopHole = true;
            currentPoints += 300;
        }
        // Mid hole
        else if (!scoredMidHole && SDL_HasIntersection(&ballRect, &midHoleRect))
        {
            scoredMidHole = true;
            currentPoints += 200;
        }
        // Bottom hole
        else if (!scoredBottomHole && SDL_HasIntersection(&ballRect, &bottomHoleRect))
        {
            scoredBottomHole = true;
            currentPoints += 100;
        }

        // Level complete if scored any hole
        if (scoredTopHole || scoredMidHole || scoredBottomHole)
        {
            showGoalCircle = false;
            ballInMotion = false;
            levelComplete = true;
            int bonus = lives * 50;
            int gained = (scoredTopHole ? 300 : scoredMidHole ? 200
                                                              : 100) +
                         bonus;
            lastGainedPoints = gained;
            lastLifeBonus = bonus;
            currentPoints += bonus;

            // High score logic
            if (currentPoints > highScore)
            {
                highScore = currentPoints;
                game->setScoreForPlayer(playerName, highScore, 3);
            }
            game->setLevelCompleted(3);
            return;
        }

        // FAIL: ball missed all holes (out of bounds or ground)
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

void ScreenLevel3::render()
{
    // showGoalCircle = true;
    // Draw background
    if (backgroundTexture)
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &backgroundRect);

    // Draw ground
    if (groundTexture)
    {
        SDL_RenderCopy(renderer, groundTexture, NULL, &ground1);
        SDL_RenderCopy(renderer, groundTexture, NULL, &ground2);
    }

    // Draw triple goal with 3 holes
    if (showGoalCircle && goalCircle3Texture)
    {
        // Draw PNG
        SDL_RenderCopy(renderer, goalCircle3Texture, NULL, &goalCircle3Rect);

        /* Draw hole hitboxes for debug
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 130); // top = red
        SDL_RenderDrawRect(renderer, &topHoleRect);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 130); // mid = green
        SDL_RenderDrawRect(renderer, &midHoleRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 130); // bot = blue
        SDL_RenderDrawRect(renderer, &bottomHoleRect);
        */

        // Draw points label for each hole
        TTF_Font *ringFont = TTF_OpenFont("../assets/font.ttf", 24);
        if (ringFont)
        {
            SDL_Color color = {255, 255, 255, 255};
            // Top: 300
            SDL_Surface *surf1 = TTF_RenderText_Solid(ringFont, "300", color);
            // Mid: 200
            SDL_Surface *surf2 = TTF_RenderText_Solid(ringFont, "200", color);
            // Bottom: 100
            SDL_Surface *surf3 = TTF_RenderText_Solid(ringFont, "100", color);

            if (surf1)
            {
                SDL_Texture *txt1 = SDL_CreateTextureFromSurface(renderer, surf1);
                SDL_Rect dst1 = {topHoleRect.x + topHoleRect.w / 2 - surf1->w / 2, topHoleRect.y - 30, surf1->w, surf1->h};
                SDL_RenderCopy(renderer, txt1, NULL, &dst1);
                SDL_FreeSurface(surf1);
                SDL_DestroyTexture(txt1);
            }
            if (surf2)
            {
                SDL_Texture *txt2 = SDL_CreateTextureFromSurface(renderer, surf2);
                SDL_Rect dst2 = {midHoleRect.x + midHoleRect.w / 2 - surf2->w / 2, midHoleRect.y - 30, surf2->w, surf2->h};
                SDL_RenderCopy(renderer, txt2, NULL, &dst2);
                SDL_FreeSurface(surf2);
                SDL_DestroyTexture(txt2);
            }
            if (surf3)
            {
                SDL_Texture *txt3 = SDL_CreateTextureFromSurface(renderer, surf3);
                SDL_Rect dst3 = {bottomHoleRect.x + bottomHoleRect.w / 2 - surf3->w / 2, bottomHoleRect.y + bottomHoleRect.h + 10, surf3->w, surf3->h};
                SDL_RenderCopy(renderer, txt3, NULL, &dst3);
                SDL_FreeSurface(surf3);
                SDL_DestroyTexture(txt3);
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

    // Draw attempt message if it is the case
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
        for (int i = obstaclesCleared; i < maxObstacles; ++i)
        {
            Obstacle &obs = obstacles[i];
            SDL_Texture *tex = nullptr;
            if (obs.type == 1)
                tex = obstacleTexture1;
            else if (obs.type == 2)
                tex = obstacleTexture2;
            else if (obs.type == 3)
                tex = obstacleTexture3;
            else if (obs.type == 4)
                tex = obstacleTexture4;
            else if (obs.type == 5)
                tex = obstacleTexture5;
            if (tex)
                SDL_RenderCopy(renderer, tex, NULL, &obs.rect);
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

        /*Draw player collision box (debug)
        SDL_Rect playerRect = {playerX, playerY - 15, frameWidth + 40, frameHeight + 100};
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 180); Magenta, semi-transparent
        SDL_RenderDrawRect(renderer, &playerRect);
        */
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
        return;
    }

    // LEVEL COMPLETE (Congratulations)
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
            SDL_Surface *surf = TTF_RenderText_Solid(font, "LEVEL 3 COMPLETE!", color);
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

            // HOME button only (no next level)
            SDL_Rect homeBtnRect = {windowWidth / 2 - 100, windowHeight / 2 + 40, 200, 55};
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
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

void ScreenLevel3::handleEvent(const SDL_Event &e)
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
                game->saveProgress(3);
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
                game->saveProgress(3); // Save progress (level 3)
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

    // LEVEL COMPLETE: ONLY HOME BUTTON
    if (levelComplete)
    {
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            int mx = e.button.x;
            int my = e.button.y;
            SDL_Rect homeBtnRect = {windowWidth / 2 - 100, windowHeight / 2 + 40, 200, 55};
            SDL_Point mousePoint = {mx, my};
            if (SDL_PointInRect(&mousePoint, &homeBtnRect))
            {
                game->clearProgress();
                game->setState(GameState::HOME);
                levelComplete = false;
                return;
            }
        }

        if (e.type == SDL_MOUSEMOTION)
        {
            int mx = e.motion.x, my = e.motion.y;
            SDL_Point mousePoint = {mx, my};
            SDL_Rect homeBtnRect = {windowWidth / 2 - 100, windowHeight / 2 + 40, 200, 55};
            congratsHomeHovered = SDL_PointInRect(&mousePoint, &homeBtnRect);
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