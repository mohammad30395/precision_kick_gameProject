#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <map>

using namespace std;

class ScreenHome;
class ScreenLevelSelect;
class ScreenLevel1;
class ScreenLevel2;
class ScreenLevel3;
class ScreenNameInput;
class ScreenHelp;         
class ScreenCredits;      
class ScreenHighestScore;

// Enum to keep track of the current game state
enum class GameState
{
    HOME,
    NAME_INPUT,
    LEVEL_SELECT,
    LEVEL_1,
    LEVEL_1_COMPLETE,
    LEVEL_2,
    LEVEL_2_COMPLETE,
    LEVEL_3,
    LEVEL_3_COMPLETE,
    HELP,    
    CREDITS, 
    HIGHEST_SCORE,
    QUIT
};

// Player progress struct for continue functionality
struct PlayerProgress
{
    string name;
    int level; // 1-based
    bool inProgress;

    PlayerProgress() : name(""), level(1), inProgress(false) {}
};

class Game
{
public:
    Game(int winWidth, int winHeight);
    ~Game();

    bool init();
    void run();
    void cleanup();

    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }
    SDL_Renderer *getRenderer() const { return renderer; }

    void setState(GameState newState);
    GameState getState() const { return state; }

    // Progress and name accessors
    void saveProgress(int level);
    void clearProgress();
    const PlayerProgress &getProgress() const { return progress; }

    // Player name 
    string getPlayerName() const;

    void setHomeShowContinue(bool show);

    void setLevelCompleted(int level);

    void setScoreForPlayer(const string &playerName, int score, int level);

    pair<string, int> getHighestScorePlayer() const;

private:
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *fontTitle;
    TTF_Font *fontButton;
    TTF_Font *fontLoading;

    ScreenHome *home;
    ScreenNameInput *nameInput;
    string playerName;
    ScreenLevelSelect *levelSel;
    ScreenLevel1 *level1;
    ScreenLevel2 *level2;
    ScreenLevel3 *level3;
    ScreenHelp *helpScreen;       
    ScreenCredits *creditsScreen; 
    ScreenHighestScore *highestScoreScreen;

    GameState state;
    int windowWidth;
    int windowHeight;

    // For Loading Screen
    bool loading;
    float loadingProgress;
    Uint32 loadingStartTime;
    const Uint32 LOADING_DURATION = 1000;

    Mix_Music *bgMusic;

    // for music icon
    bool musicOn;
    SDL_Texture *iconMusicOn;
    SDL_Texture *iconMusicOff;
    SDL_Rect musicIconRect;

    //  Progress tracking for continue functionality 
    PlayerProgress progress;
    // Map to store high scores per player, per level
    map<string, map<int, int>> playerScores;
    void savePlayerScoresToFile();
    void loadPlayerScoresFromFile();
};