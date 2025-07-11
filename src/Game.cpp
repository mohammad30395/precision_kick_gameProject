#include "../include/Game.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "../include/ScreenLevel1.h"
#include "../include/ScreenLevel2.h"
#include "../include/ScreenLevel3.h"
#include "../include/ScreenNameInput.h"
#include "../include/ScreenHome.h"
#include "../include/ScreenLevelSelect.h"
#include "../include/ScreenHelp.h"
#include "../include/ScreenCredits.h"
#include "../include/ScreenHighestScore.h"
#include <fstream>
#include <sstream>

#include <iostream>
using namespace std;

// Loading duration
const Uint32 LOADING_DURATION = 1000; // 1 second

// Loads image and creates an SDL_Texture
static SDL_Texture *loadTexture(SDL_Renderer *renderer, const string &path)
{
    SDL_Surface *surf = IMG_Load(path.c_str()); // loads the image file into an sdl Surface
    if (!surf)
    {
        cout << "IMG_Load Error: " << IMG_GetError() << endl;
        return nullptr;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf); // convert surface into sdl texture
    SDL_FreeSurface(surf);
    return tex;
}

// game constructor. Initializes all member pointers to null, all the members of game.h
Game::Game(int winWidth, int winHeight)
    : window(nullptr), renderer(nullptr), fontTitle(nullptr), fontButton(nullptr), fontLoading(nullptr),
      home(nullptr), levelSel(nullptr), level1(nullptr), level2(nullptr), level3(nullptr),
      nameInput(nullptr), helpScreen(nullptr), creditsScreen(nullptr), highestScoreScreen(nullptr),
      state(GameState::HOME), windowWidth(winWidth), windowHeight(winHeight),
      loading(false), loadingProgress(0.0f), loadingStartTime(0),
      bgMusic(nullptr),
      musicOn(true), iconMusicOn(nullptr), iconMusicOff(nullptr),
      playerName("")
{
    int iconSize = 64; // sets up music icon
    int margin = 30;
    musicIconRect = {winWidth - iconSize - margin, margin, iconSize, iconSize};
}

Game::~Game() { cleanup(); } // destructor calls cleanup

// initialize SDL, TTF, IMG, and SDL_mixer.
// loads all fonts textures and music, and sets up screen objects.
bool Game::init()
{
    loadPlayerScoresFromFile();

    if (TTF_Init() == -1)
    {
        cout << "TTF Error: " << TTF_GetError() << endl;
        return false;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        cout << "IMG Error: " << IMG_GetError() << endl;
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        cout << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow("PRECISION KICK", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    if (!window)
    {
        cout << "Window Error: " << SDL_GetError() << endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        cout << "Renderer Error: " << SDL_GetError() << endl;
        return false;
    }

    fontTitle = TTF_OpenFont("../assets/font.ttf", 80);
    if (!fontTitle)
    {
        cout << "FontTitle Error: " << TTF_GetError() << endl;
        return false;
    }
    fontButton = TTF_OpenFont("../assets/font.ttf", 44);
    if (!fontButton)
    {
        cout << "FontButton Error: " << TTF_GetError() << endl;
        return false;
    }
    fontLoading = TTF_OpenFont("../assets/font.ttf", 38);
    if (!fontLoading)
    {
        fontLoading = fontButton;
    }

    bgMusic = Mix_LoadMUS("../assets/magic_system.mp3");
    if (!bgMusic)
    {
        cout << "Failed to load background music! Mix_Error: " << Mix_GetError() << endl;
    }

    iconMusicOn = loadTexture(renderer, "../assets/music_on.png");
    iconMusicOff = loadTexture(renderer, "../assets/music_off.png");
    if (!iconMusicOn || !iconMusicOff)
    {
        cout << "Failed to load music icons!" << endl;
    }

    musicOn = true;
    if (bgMusic)
        Mix_PlayMusic(bgMusic, -1);

    home = new ScreenHome(renderer, fontTitle, fontButton, windowWidth, windowHeight);
    levelSel = new ScreenLevelSelect(renderer, fontTitle, fontButton, windowWidth, windowHeight);
    level1 = nullptr;
    level2 = nullptr;
    level3 = nullptr;
    nameInput = new ScreenNameInput(renderer, fontTitle, fontButton, windowWidth, windowHeight);

    helpScreen = new ScreenHelp(renderer, fontTitle, fontButton, windowWidth, windowHeight);
    creditsScreen = new ScreenCredits(renderer, fontTitle, fontButton, windowWidth, windowHeight);
    highestScoreScreen = new ScreenHighestScore(renderer, fontTitle, fontButton, windowWidth, windowHeight);

    state = GameState::HOME;
    loading = false;
    loadingProgress = 0.0f;
    loadingStartTime = 0;

    // if game is unfinished, show continue
    setHomeShowContinue(progress.inProgress);

    return true;
}

void Game::setState(GameState newState) // switch the current game state
{
    // Delete level 1 screen if leaving level 1
    if ((state == GameState::LEVEL_1 || state == GameState::LEVEL_1_COMPLETE) &&
        (newState != GameState::LEVEL_1 && newState != GameState::LEVEL_1_COMPLETE) && level1)
    {
        saveProgress(1);
        delete level1;
        level1 = nullptr;
    }
    // Delete level 2 screen if leaving level 2
    if ((state == GameState::LEVEL_2 || state == GameState::LEVEL_2_COMPLETE) &&
        (newState != GameState::LEVEL_2 && newState != GameState::LEVEL_2_COMPLETE) && level2)
    {
        saveProgress(2);
        delete level2;
        level2 = nullptr;
    }
    // Delete level 3 screen if leaving level 3
    if ((state == GameState::LEVEL_3 || state == GameState::LEVEL_3_COMPLETE) &&
        (newState != GameState::LEVEL_3 && newState != GameState::LEVEL_3_COMPLETE) && level3)
    {
        saveProgress(3);
        delete level3;
        level3 = nullptr;
    }

    state = newState; // set new state

    string pname = progress.inProgress ? progress.name : playerName; // getting current player name

    // Cration of level 1 screen if needed
    if (state == GameState::LEVEL_1)
    {
        if (level1)
            delete level1;
        level1 = new ScreenLevel1(this, renderer, windowWidth, windowHeight);
        level1->setPlayerName(pname);
        level1->setHighScore(playerScores[pname][1]);
    }
    // creation of level 2 screen if needed
    if (state == GameState::LEVEL_2)
    {
        if (level2)
            delete level2;
        level2 = new ScreenLevel2(this, renderer, windowWidth, windowHeight);
        level2->setPlayerName(pname);
        level2->setHighScore(playerScores[pname][2]);
    }
    // Creatuon of level 3 screen if needed
    if (state == GameState::LEVEL_3)
    {
        if (level3)
            delete level3;
        level3 = new ScreenLevel3(this, renderer, windowWidth, windowHeight);
        level3->setPlayerName(pname);
        level3->setHighScore(playerScores[pname][3]);
    }

    // Reset name input if entering that screen
    if (state == GameState::NAME_INPUT && nameInput)
    {
        nameInput->reset();
    }
}

// Save the player's name and highest reached level
void Game::saveProgress(int level)
{
    progress.name = playerName;                  // Set player name in progress
    progress.level = max(progress.level, level); // Update to highest level reached
    progress.inProgress = true;
    setHomeShowContinue(true); // Show continue button on home
}

// reset all progress data for new game
void Game::clearProgress()
{
    progress = PlayerProgress(); // reset progress
    setHomeShowContinue(false);
}

// Return the current player name
string Game::getPlayerName() const
{
    return progress.inProgress ? progress.name : playerName; // Using in-progress name if set
}

// handling game in running state
void Game::run()
{
    bool quit = false; // exit flag
    SDL_Event e;

    while (!quit && state != GameState::QUIT)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) // quit if window closed
            {
                quit = true;
                state = GameState::QUIT;
            }

            // Music icon toggle
            if ((state == GameState::HOME || state == GameState::LEVEL_SELECT || state == GameState::NAME_INPUT || state == GameState::HELP || state == GameState::CREDITS || state == GameState::HIGHEST_SCORE) &&
                e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
            {
                int mx = e.button.x, my = e.button.y;
                if (mx >= musicIconRect.x && mx <= musicIconRect.x + musicIconRect.w &&
                    my >= musicIconRect.y && my <= musicIconRect.y + musicIconRect.h)
                {
                    musicOn = !musicOn;
                    if (musicOn && bgMusic)
                        Mix_PlayMusic(bgMusic, -1);
                    else
                        Mix_HaltMusic();
                }
            }

            // Event handling based on state
            if (state == GameState::HOME && !loading)
            {
                int act = home->handleEvent(e);
                bool canContinue = home->getShowContinue();

                if (canContinue)
                {
                    if (act == 0) // continue
                    {
                        playerName = progress.name;
                        if (progress.level >= 3)
                            setState(GameState::LEVEL_3);
                        else if (progress.level == 2)
                            setState(GameState::LEVEL_2);
                        else
                            setState(GameState::LEVEL_1);
                    }
                    else if (act == 1) // new game
                    {
                        clearProgress();
                        loading = true;
                        loadingStartTime = SDL_GetTicks();
                        loadingProgress = 0.0f;
                    }
                    else if (act == 2)
                        setState(GameState::HIGHEST_SCORE);
                    else if (act == 3)
                        setState(GameState::HELP);
                    else if (act == 4)
                        setState(GameState::CREDITS);
                    else if (act == 5)
                    {
                        quit = true;
                        state = GameState::QUIT;
                    }
                }
                else
                {
                    if (act == 0) // new game
                    {
                        clearProgress();
                        loading = true;
                        loadingStartTime = SDL_GetTicks();
                        loadingProgress = 0.0f;
                    }
                    else if (act == 1)
                        setState(GameState::HIGHEST_SCORE);
                    else if (act == 2)
                        setState(GameState::HELP);
                    else if (act == 3)
                        setState(GameState::CREDITS);
                    else if (act == 4)
                    {
                        quit = true;
                        state = GameState::QUIT;
                    }
                }
            }
            else if (state == GameState::HELP)
            {
                bool back = helpScreen->handleEvent(e);
                if (back)
                    setState(GameState::HOME);
            }
            else if (state == GameState::CREDITS)
            {
                bool back = creditsScreen->handleEvent(e);
                if (back)
                    setState(GameState::HOME);
            }
            else if (state == GameState::NAME_INPUT)
            {
                int result = nameInput->handleEvent(e, playerName);
                if (result == 1)
                {
                    playerName = nameInput->getName();
                    saveProgress(1);
                    setState(GameState::LEVEL_SELECT);
                }
                else if (result == -1)
                    setState(GameState::HOME);
            }
            else if (state == GameState::LEVEL_SELECT)
            {
                if (levelSel->backClicked(e))
                {
                    setState(GameState::HOME);
                }
                int act = levelSel->handleEvent(e);
                if (act == 0)
                    setState(GameState::LEVEL_1);
                if (act == 1)
                    setState(GameState::LEVEL_2);
                if (act == 2)
                    setState(GameState::LEVEL_3);
            }
            else if (state == GameState::LEVEL_1 || state == GameState::LEVEL_1_COMPLETE)
            {
                if (level1)
                    level1->handleEvent(e);
            }
            else if (state == GameState::LEVEL_2 || state == GameState::LEVEL_2_COMPLETE)
            {
                if (level2)
                    level2->handleEvent(e);
            }
            else if (state == GameState::LEVEL_3 || state == GameState::LEVEL_3_COMPLETE)
            {
                if (level3)
                    level3->handleEvent(e);
            }
            else if (state == GameState::HIGHEST_SCORE)
            {
                if (highestScoreScreen->handleEvent(e))
                {
                    setState(GameState::HOME);
                }
            }
        }

        // Loading bar logic
        if (state == GameState::HOME && loading)
        {
            Uint32 elapsed = SDL_GetTicks() - loadingStartTime;
            loadingProgress = (float)elapsed / LOADING_DURATION;
            if (loadingProgress >= 1.0f)
            {
                loadingProgress = 1.0f;
                loading = false;
                setState(GameState::NAME_INPUT);
            }
        }

        // music control
        if (state == GameState::HOME || state == GameState::LEVEL_SELECT ||
            state == GameState::NAME_INPUT || state == GameState::HELP || state == GameState::CREDITS || state == GameState::HIGHEST_SCORE)
        {
            if (musicOn && bgMusic && !Mix_PlayingMusic())
            {
                Mix_PlayMusic(bgMusic, -1);
            }
            if (!musicOn && Mix_PlayingMusic())
            {
                Mix_HaltMusic();
            }
        }
        else
        {
            if (Mix_PlayingMusic())
            {
                Mix_HaltMusic();
            }
        }

        // rendering objects
        SDL_RenderClear(renderer);
        if (state == GameState::HOME)
        {
            home->render(progress.inProgress ? progress.name : "");
            if (loading)
            {
                int boxW = windowWidth / 2 + 80;
                int boxH = windowHeight / 28;
                int marginBottom = 42;
                int boxX = windowWidth / 2 - boxW / 2;
                int boxY = windowHeight - marginBottom - boxH;
                SDL_Rect outlineRect = {boxX, boxY, boxW, boxH};
                SDL_SetRenderDrawColor(renderer, 20, 40, 80, 230);
                SDL_RenderDrawRect(renderer, &outlineRect);

                int barMargin = 6;
                int barHeight = boxH - 2 * barMargin;
                int barWidth = static_cast<int>((boxW - 2 * barMargin) * loadingProgress);
                SDL_Rect barRect = {boxX + barMargin, boxY + barMargin, barWidth, barHeight};
                SDL_SetRenderDrawColor(renderer, 0, 0, 80, 255);
                SDL_RenderFillRect(renderer, &barRect);
            }
            SDL_Texture *musicIcon = musicOn ? iconMusicOn : iconMusicOff;
            if (musicIcon)
                SDL_RenderCopy(renderer, musicIcon, nullptr, &musicIconRect);
        }
        else if (state == GameState::HELP)
        {
            helpScreen->render();
            SDL_Texture *musicIcon = musicOn ? iconMusicOn : iconMusicOff;
            if (musicIcon)
                SDL_RenderCopy(renderer, musicIcon, nullptr, &musicIconRect);
        }
        else if (state == GameState::CREDITS)
        {
            creditsScreen->render();
            SDL_Texture *musicIcon = musicOn ? iconMusicOn : iconMusicOff;
            if (musicIcon)
                SDL_RenderCopy(renderer, musicIcon, nullptr, &musicIconRect);
        }
        else if (state == GameState::NAME_INPUT)
        {
            nameInput->render();
            SDL_Texture *musicIcon = musicOn ? iconMusicOn : iconMusicOff;
            if (musicIcon)
                SDL_RenderCopy(renderer, musicIcon, nullptr, &musicIconRect);
        }
        else if (state == GameState::LEVEL_SELECT)
        {
            levelSel->render();
            SDL_Texture *musicIcon = musicOn ? iconMusicOn : iconMusicOff;
            if (musicIcon)
                SDL_RenderCopy(renderer, musicIcon, nullptr, &musicIconRect);
        }
        else if (state == GameState::LEVEL_1 || state == GameState::LEVEL_1_COMPLETE)
        {
            if (level1)
            {
                level1->update();
                level1->render();
            }
        }
        else if (state == GameState::LEVEL_2 || state == GameState::LEVEL_2_COMPLETE)
        {
            if (level2)
            {
                level2->update();
                level2->render();
            }
        }
        else if (state == GameState::LEVEL_3 || state == GameState::LEVEL_3_COMPLETE)
        {
            if (level3)
            {
                level3->update();
                level3->render();
            }
        }
        else if (state == GameState::HIGHEST_SCORE)
        {
            // Get the player with the highest total score
            auto [pname, maxScore] = getHighestScorePlayer();

            // Gathering per-level scores for this player
            vector<int> scores(3, 0);
            int totalScore = 0;
            if (!pname.empty())
            {
                for (int i = 1; i <= 3; ++i)
                {
                    int pts = 0;
                    if (playerScores.count(pname) && playerScores[pname].count(i))
                        pts = playerScores[pname][i];
                    scores[i - 1] = pts;
                    totalScore += pts;
                }
            }
            highestScoreScreen->render(pname, scores, totalScore);
            SDL_Texture *musicIcon = musicOn ? iconMusicOn : iconMusicOff;
            if (musicIcon)
                SDL_RenderCopy(renderer, musicIcon, nullptr, &musicIconRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

void Game::setLevelCompleted(int level)
{
    progress.name = playerName;
    progress.level = max(progress.level, level);
    progress.inProgress = true;
    setHomeShowContinue(true);

    // Setting game state to the correct level completion state
    if (level == 1)
        state = GameState::LEVEL_1_COMPLETE;
    else if (level == 2)
        state = GameState::LEVEL_2_COMPLETE;
    else if (level == 3)
        state = GameState::LEVEL_3_COMPLETE;
}

// freeing resources and memory for avoiding leaks
void Game::cleanup()
{
    savePlayerScoresToFile(); // Save all player scores to dat file
    if (home)
    {
        delete home;
        home = nullptr;
    }
    if (levelSel)
    {
        delete levelSel;
        levelSel = nullptr;
    }
    if (level1)
    {
        delete level1;
        level1 = nullptr;
    }
    if (level2)
    {
        delete level2;
        level2 = nullptr;
    }
    if (level3)
    {
        delete level3;
        level3 = nullptr;
    }
    if (helpScreen)
    {
        delete helpScreen;
        helpScreen = nullptr;
    }
    if (creditsScreen)
    {
        delete creditsScreen;
        creditsScreen = nullptr;
    }
    if (highestScoreScreen)
    {
        delete highestScoreScreen;
        highestScoreScreen = nullptr;
    }
    if (nameInput)
    {
        delete nameInput;
        nameInput = nullptr;
    }
    if (fontTitle)
    {
        TTF_CloseFont(fontTitle);
        fontTitle = nullptr;
    }
    if (fontButton)
    {
        TTF_CloseFont(fontButton);
        fontButton = nullptr;
    }
    if (fontLoading && fontLoading != fontButton)
    {
        TTF_CloseFont(fontLoading);
        fontLoading = nullptr;
    }
    if (iconMusicOn)
    {
        SDL_DestroyTexture(iconMusicOn);
        iconMusicOn = nullptr;
    }
    if (iconMusicOff)
    {
        SDL_DestroyTexture(iconMusicOff);
        iconMusicOff = nullptr;
    }
    if (bgMusic)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(bgMusic);
        bgMusic = nullptr;
    }
    Mix_CloseAudio();
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (highestScoreScreen)
    {
        cout << "Deleting highestScoreScreen: " << highestScoreScreen << endl;
        delete highestScoreScreen;
        highestScoreScreen = nullptr;
    }
}

void Game::setHomeShowContinue(bool show)
{
    if (home)
        home->setShowContinue(show);
}

// Updates the player’s best score for a given level if the new score is higher than their previous best
void Game::setScoreForPlayer(const string &playerName, int score, int level)
{
    if (playerScores[playerName][level] < score)
        playerScores[playerName][level] = score;

    savePlayerScoresToFile();
}

// Writes every player’s best score for each level to a dat file
void Game::savePlayerScoresToFile()
{
    ofstream out("player_scores.dat"); // Open file for writing
    for (const auto &[name, levelScores] : playerScores)
    {
        out << name << "\n";

        // Write player's best score for each level
        for (int i = 1; i <= 3; ++i)
        {
            int score = 0;
            if (levelScores.count(i))
                score = levelScores.at(i);
            out << score << " ";
        }
        out << "\n";
    }
    out.close();
}

// Reads every player’s per-level scores from dat file and loads them in memory at game startup.
void Game::loadPlayerScoresFromFile()
{
    ifstream in("player_scores.dat"); // Open score file for reading
    if (!in)
        return;
    playerScores.clear();
    string name;
    while (getline(in, name))
    {
        string line;
        if (!getline(in, line))
            break;
        istringstream iss(line);
        int s1, s2, s3;
        iss >> s1 >> s2 >> s3; // reads three level scores
        playerScores[name][1] = s1;
        playerScores[name][2] = s2;
        playerScores[name][3] = s3;
    }
    in.close();
}

// Finds the player who has the highest total score
pair<string, int> Game::getHighestScorePlayer() const
{
    string bestPlayer;
    int maxScore = 0;
    for (const auto &entry : playerScores)
    {
        int total = 0;
        for (const auto &levelScore : entry.second)
            total += levelScore.second;
        if (total > maxScore)
        {
            maxScore = total;
            bestPlayer = entry.first;
        }
    }
    return {bestPlayer, maxScore};
}