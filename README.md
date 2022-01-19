# CPPND: Capstone Snake Game... Evolved!

This is a C++ program developed for the final Capstone project in the [Udacity C++ Nanodegree Program](https://www.udacity.com/course/c-plus-plus-nanodegree--nd213). 
The program is based on the [Starter Snake Game Repo](https://github.com/udacity/CppND-Capstone-Snake-Game), which was expanded with new features, such as the addition of a A.I. learning mode.

<img src="snake_game_evolved.png"/>

## New Game Features

1. Auto Mode: press 'A' key at any time during gameplay to toggle back and forth between manual (standard) and a new auto (A.I.) mode (a confirmation pop-up window will appear first). In the latter, the snake is controlled by the CPU, instead of the player, and gradually improves at the game over time - making it more challenging for the player to surpass the CPU record score. Game starts in manual mode by default.
2. Multi-layer Perceptron (MLP) Neural Network used as the CPU's decision model for the snake direction during auto mode, while a Genetic Algorithm is used for the training of the MLP weights (i.e. every generation, the MLP weight sets resulting in the best game scores are selected to breed the next weights sets population).
3. Speed Mode: only available during auto (A.I.) mode. Press 'S' key to toggle back and forth between frame rate control (60 fps) and no frame rate control, where the game runs much faster, in order to allow faster A.I. learning process.
4. Game Pause: press 'P' key to pause the game (and resume) during any of the modes (manual or auto).
5. Game Replay: when in manual (standard) mode, after the snake dies, the player is asked if he wants to play another game round through a pop-up window. On the other hand, in auto (A.I.) mode, no confirmation is needed, and new rounds start automatically after the snake dies.
6. Infinite loop prevention during Auto Mode: during auto mode, as the MLP weights are not adapted throughout a game round (and only -between- game rounds), the CPU decisions will be deterministic depending on the snake position and direction in the grid. So, in order to prevent the game to be stuck in a loop, in case a specific snake's position+direction set is reprised before the new food is eaten, the snake is automatically killed.
7. Special Congratulating Messages: when the player achieves a new record score, or when the game is completed (i.e. all grid spaces filled with the snake body), a special congratulating message box appears.
8. Saved game data: both player and CPU score records and A.I. learnings are automatically saved between program executions. If the user wants to reset everything and erase data, 'E' key can be pressed at any time (a confirmation pop-up window will appear first).
9. Improved randomization algorithm for the appearance of new food, considering now only the empty positions in the game grid.
10. Collidable walls on the grid boundaries.

## Game Controls

The game controls are the following (they are also displayed in an initial game message window):
1. Arrow keys: controls the snake in Manual mode (i.e. player in control);
2. 'A' key: toggles Auto mode (i.e. CPU in control) on and off;
3. 'S' key: toggles Speed mode (i.e. no frame rate control) on and off (obs.: only available in Auto mode);
4. 'P' key: pauses the game (or resumes it);
5. 'E' key: erases and resets all game data, inclusing record scores and A.I. learning;
6. Close game window: ends the game and exit.

## Dependencies for Running Locally
* OS: Linux
* cmake >= 3.7
  * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1 (Linux)
  * Linux: make is installed by default on most Linux distros
* SDL2 >= 2.0
  * All installation instructions can be found [here](https://wiki.libsdl.org/Installation)
  * Note that for Linux, an `apt` or `apt-get` installation is preferred to building from source.
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
* Eigen C++ Library (Linear Algebra Library)
  * Linux: already included in the project "lib" folder. [More info on the library here.](https://eigen.tuxfamily.org/index.php?title=Main_Page)

## Basic Build Instructions

On Linux:
1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./SnakeGame`.
