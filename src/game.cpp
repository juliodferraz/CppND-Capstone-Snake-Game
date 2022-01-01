#include "game.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <climits>
#include "SDL.h"
#include "clip.h"

/**
 *  \brief Clip grid size length at INT_MAX. 
 * This is done because spacial coordinates are worked under integer representation throughout this application.
 *  \param a Grid side length (unsigned int value).
 *  \return Clipped grid side length (unsigned int value).
 */
#define CLIP_GRID_SIDE_LEN(a) ((unsigned int) std::min<unsigned int>(a, (unsigned int) INT_MAX))

Game::Game(const unsigned int winWidth, const unsigned int winHeight, const unsigned int gridSideLen) 
  : renderer(winWidth, winHeight, CLIP_GRID_SIDE_LEN(gridSideLen)),
    world(CLIP_GRID_SIDE_LEN(gridSideLen)),
    snake(SDL_Point{(int) CLIP_GRID_SIDE_LEN(gridSideLen)/2, (int) CLIP_GRID_SIDE_LEN(gridSideLen)/2}, world) {}

void Game::Run(const unsigned int targetFramePeriod, const unsigned int timeLimitInFrames) {
  // Try to load previous game state from save file, in case there's one available.
  // If not, game will start from beginning.
  LoadSaveFile();

  // Set game running state to true.
  running = true;

  // Declare and initialize variables to be used in the game main loop.
  unsigned int titleUpdTimestamp = SDL_GetTicks();
  unsigned int frameStart;
  unsigned int frameEnd;
  unsigned int frameDuration;
  unsigned int frameRateCnt = 0;

  // Main game loop, while running flag is still true
  while (running) {
    // Store the timestamp for the beginning of the frame.
    frameStart = SDL_GetTicks();

    // Receive Input, Update, Render - the main game loop
    UpdateState(controller.ReceiveCommand());
    renderer.Render(world, snake);

    // Increment the fps count.
    frameRateCnt = CLPD_UINT_SUM(frameRateCnt, 1);

    // Get the timestamp after all game update and rendering processing has finished.
    frameEnd = SDL_GetTicks();

    // Calculate the duration of the last frame.
    frameDuration = CLPD_UINT_DIFF(frameEnd, frameStart);

    // After every second, update the window title.
    if ( CLPD_UINT_DIFF(frameEnd, titleUpdTimestamp) >= 1000) {
      renderer.UpdateWindowTitle(this->GetScore(), frameRateCnt, snake.IsAutoModeOn(), 
        timeLimitInFrames-this->timeLimitFrameCnt,
        snake.GetGenAlgGeneration(), snake.GetGenAlgIndividual(), this->maxScore);

      // Reset the fps count.
      frameRateCnt = 0;

      // Reset the title update timestamp.
      titleUpdTimestamp = frameEnd;
    }

    // If the time for the current frame was too small (i.e. frameDuration is smaller than the target frame period), 
    // delay the loop to achieve the correct frame rate.
    // This is only done if frame rate control is active.
    if (fpsCtrlActv == true && frameDuration < targetFramePeriod) {
      SDL_Delay(CLPD_UINT_DIFF(targetFramePeriod, frameDuration));
    }

    // Keep track of the game time limit (which resets everytime the snake eats).
    // This is done to prevent the snake to be stuck in an infinite loop during the Genetic Algorithm learning.
    timeLimitFrameCnt = CLPD_UINT_SUM(timeLimitFrameCnt, 1);
    if (timeLimitFrameCnt > timeLimitInFrames) {
      // If the time limit has been surpassed, kill the snake, which will lead to the end of the game round.
      snake.SetEvent(Snake::Event::Killed);
    }

    // If the snake is dead or player has won the game, do the necessary processing in order to start a new game round.
    if (!snake.IsAlive() || victory) {
      // Try to update the maximum game score, in case a record was achieved.
      maxScore = std::max(this->maxScore, this->GetScore());

      // Update the fitness of the current individual in the Genetic Algorithm population, which then moves it to the next individual.
      // Set the fitness as equal to the snake size/score.
      // Obs.: this is only done in case the CPU controlled the snake from start to finish of the game, otherwise the
      // result doesn't count for the snake AI learning (in case the snake was controlled by the player at any point of time).
      snake.GradeFitness((float) snake.GetSize());

      // Reset the game and start a new round.
      Reset();
    }
  }

  // Stores the game state in a save file, for it to be resumed in the next execution.
  StoreSaveFile();
}

void Game::UpdateState(const Controller::UserCommand command) {
  // First, process the input user command.
  if (command == Controller::UserCommand::Quit) {
    running = false;
  } else if (command == Controller::UserCommand::ToggleFpsCtrl) {
    fpsCtrlActv = !fpsCtrlActv;
  } else {
    // For any other command, pass the command on to be processed by the snake object.
    snake.ProcessUserCommand(command);
  }

  // Next, update the game state accordingly.
  // If the snake is deceased, no world update needs to be done.
  if (!snake.IsAlive()) return;

  // Otherwise, move the snake in its current direction.
  snake.Move();

  // Check if snake head is about to move to a new tile.
  SDL_Point targetHeadPosition{snake.GetTargetHeadPosition()};
  SDL_Point headPosition{snake.GetHeadPosition()};

  if (!(targetHeadPosition == headPosition)) {
    // Checks the new tile content and raises appropriate event (e.g. eating, collision, etc.)
    if (world.IsObstacle(targetHeadPosition)) {
      snake.SetEvent(Snake::Event::Killed);

    } else {
      if (world.GetElement(targetHeadPosition) == World::Element::Food) {
        snake.SetEvent(Snake::Event::Ate);

        // Everytime the snake eats, reset the time limit frame count.
        this->timeLimitFrameCnt = 0;

        // Now that the food has been eaten, make new food appear in a free grid tile.
        if (!world.GrowFood()) {
          // If a new food cannot be placed, the game has been won.
          this->victory = true;
        }
        
      } else {
        // If the snake hasn't collided or eaten, just move it to the new tile.
        snake.SetEvent(Snake::Event::NewTile);
      }

      // If the snake is on automatic mode, call its decision model in order to define the next action/direction.
      if (snake.IsAutoModeOn()) {
        snake.DefineAction();
      }
    }

  } else {
    // Snake head is still in the same world grid tile.
    snake.SetEvent(Snake::Event::SameTile);
  }
}

void Game::Reset() {
  // Reinitialize the world.
  world.Init();

  // Reinitialize the snake.
  snake.Init();

  // Reset the time limit frame count.
  this->timeLimitFrameCnt = 0;

  // Reset the victory state.
  this->victory = false;
}

void Game::StoreSaveFile() const {
  // Remove previous save file, in case it exists.
  remove("../save_state.txt");

  // Create the file and open.
  std::ofstream file("../save_state.txt");

  if (file.is_open()) {
    // Store the game state.
    file << maxScore << std::endl;

    // Save the mlp configuration and genetic algorithm state.
    snake.StoreState(file);
  }

  file.close();
}

void Game::LoadSaveFile() {
  std::ifstream file("../save_state.txt");
  if (file.is_open()) {
    // Restore the game state.
    file >> maxScore;

    // Restore the mlp configuration and genetic algorithm state.
    snake.LoadState(file);
  }
  file.close();
}