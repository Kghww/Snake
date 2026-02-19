# Snake Project - Basics of Computer Programming 2024/25

## Project Overview
This is a classic "Snake" video game implementation created for the Basics of Computer Programming course. The project strictly follows the requirement of not using the C++ Standard Template Library (no `std::string`, `std::vector`, etc.). Data structures and memory management are handled using raw arrays and the C standard library.

## Implemented Features ✅
*  **GUI & Design:** Game board outline, elapsed time display, and score counter.
*  **Controls:** Arrow keys for movement, 'Esc' to exit, and 'n' for a new game. Includes automatic turning at board edges.
*  **Collision Detection:** Game ends if the snake hits the board boundaries or its own body.
*  **Info Display:** Real-time display of elapsed time and a list of implemented requirements.
*  **Snake Lengthening:** Random blue dots appear; eating them increases snake length.
*  **Speedup:** The game speed increases by a set factor after fixed time intervals.
*  **Red Dot Bonus:** Temporary red dots appear with a progress bar. Eating them either shortens the snake or slows down the game.
*  **Points System:** Score tracking for eating blue and red dots.

## Requirements & Compilation
The following files must be in the root directory for the project to work:
* **Source:** `main.cpp`
* **Assets:** `cs8x8.bmp`
* **Linux:** `libSDL2.a` / `libSDL2-64.a`, `sdl/include` folder, and `comp` / `comp64` scripts.
* **Windows:** `.vcxproj` files and `SDL2.dll`.

## How to Run

### Linux (Terminal)
1. Give permissions: `chmod +x comp64`
2. Compile: `./comp64`
3. Run: `./main`

### Windows (Visual Studio)
1. Open the `.vcxproj` file.
2. Press **F5** to compile and run.

## Potential Future Developments ⭕
*  **Fancy Graphics:** Animated body (alternating cell sizes) and pulsating dots.
*  **Teleportation:** Randomly placed pairs of numbered cells that teleport the snake.
*  **Game Configurator:** A custom text file format to load initial speed, board dimensions, and bonus frequencies.
*  **Auto Player:** An algorithm that automatically navigates the snake toward the dots using basic pathfinding.
*  **Save/Load:** 's' key saves the current game state to a file; 'l' key loads the last saved state.
*  **Best Scores:** A persistent Top 3 high-score list saved in a file, including player name input.
