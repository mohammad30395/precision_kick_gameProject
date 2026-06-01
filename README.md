# Precision Kick

**Precision Kick** is a 2D sports-action game built in C++ using the SDL2 library. It combines fast-paced platforming mechanics with precision physics-based shooting. Players must survive an obstacle-jumping phase before carefully calculating their shot velocity and angle to score a goal!

## 🎮 Gameplay

The game consists of three distinct levels, each divided into two main phases:

1. **Obstacle Phase**: The player runs automatically while obstacles approach. You must time your jumps perfectly to clear them.
2. **Shooting Phase**: After clearing a set number of obstacles, the ball phase begins. The player must hold down the **S** and **K** keys to charge the horizontal (X) and vertical (Y) velocity bars, aiming for the goal circle.

### Features
- **3 Unique Levels**: Each with increasing difficulty.
- **Player Profiles**: Enter your name to track your unique progress and high scores.
- **Score System**: Earn points for successful kicks and track the highest score per level.
- **Life System**: Players start with a set amount of lives (hearts). Hitting obstacles or missing the goal consumes a life.
- **Custom UI**: Fully fledged menu system including Home, Level Select, Pause, Help, Credits, and Leaderboards.
- **Audio**: Background music and sound effects management.

## ⌨️ Controls

### Obstacle Phase
- **Space** or **Up Arrow**: Jump over incoming obstacles.

### Shooting Phase
- **S**: Hold and release to set the horizontal (X) velocity of the kick.
- **K**: Hold and release to set the vertical (Y) velocity of the kick.

### General UI
- **Mouse**: Click on buttons to navigate menus (Play, Pause, Home, Continue, etc.).

## 🛠️ Prerequisites

To build and run this project, you will need a C++ compiler and the following SDL2 development libraries installed on your system:

- `SDL2`
- `SDL2_image`
- `SDL2_ttf`
- `SDL2_mixer`

### Linux (Debian/Ubuntu) Installation:
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

### MacOS (Using Homebrew):
```bash
brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

## 🚀 Building and Running

The project includes a `Makefile` for easy compilation.

1. Clone or download the repository.
2. Navigate to the project directory in your terminal:
   ```bash
   cd precision_kick_gameProject
   ```
3. Build the project using `make`:
   ```bash
   make
   ```
   *(To perform a clean build, you can run `make clean && make`)*
4. Run the executable:
   ```bash
   ./precision_kick
   ```

## 📂 Project Structure

- `src/` - Contains all C++ source files (`.cpp`).
- `include/` - Contains all C++ header files (`.h`).
- `assets/` - Contains game assets (fonts, sprites, backgrounds, and audio).
- `Makefile` - Build instructions.
- `player_scores.dat` - Local save data for tracking player scores.
