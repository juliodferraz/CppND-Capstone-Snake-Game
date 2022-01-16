#include "game.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include <climits>
#include "SDL.h"
#include "clip.h"
#include "config.h"

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

void Game::Run(const unsigned int targetFramePeriod) {
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
      renderer.UpdateWindowTitle(this->GetScore(), frameRateCnt, this->maxScorePlayer,
        snake.IsAutoModeOn(), this->maxScoreAI, snake.GetGenAlgGeneration(), 
        snake.GetGenAlgIndividual(), this->paused);

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

    // If the game is paused, skip automode time countdown and game round end check.
    if (!paused) {
      
      if (snake.IsAutoModeOn()) {

        // If the snake is dead or won the game, do the necessary processing in order to start a new game round.
        if (!snake.IsAlive() || victory) {
          // Try to update the maximum game score, in case a record was achieved.
          this->maxScoreAI = std::max(this->maxScoreAI, this->GetScore());

          // Update the fitness of the current individual in the Genetic Algorithm population, which then moves 
          // it to the next individual.
          // Set the fitness as equal to the snake size/score.
          // Obs.: this is only done in case the CPU controlled the snake from start to finish of the game, 
          // otherwise the result doesn't count for the snake AI learning (in case the snake was controlled by 
          // the player at any point of time).
          snake.GradeFitness((float) snake.GetSize());

          // Reset the game and start a new round.
          this->NewRound();
        }

      } else if (victory || !snake.IsAlive()) {
        // Else if snake is being controlled by the player and current round has ended...

        if (victory) {
          // If the player won the round, display a special congratulating message.
          // Try to update the record player score.
          this->maxScorePlayer = std::max(this->maxScorePlayer, this->GetScore());

          // Display victory congratulating message.
          std::string message = "Congratulations for getting past all obstacles and growing to the max," 
            " you've completed the game!! :)";
          int msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "The Snake Game", message.c_str(), NULL);
          if (msgNotOk) throw std::runtime_error("Error during display of game's victory message box.");

        } else {
          // Else if the player collided, check for the score.
          // If the score is a new record, display a special congratulating message.
          if (this->GetScore() > this->maxScorePlayer) {
            // Display congratulating message for the new game record.
            std::string message = "You've set a new record, congratulations!\nPrior player record score: " +
              std::to_string(this->maxScorePlayer) + "\nNew player record score: " + std::to_string(this->GetScore());
            int msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "The Snake Game", message.c_str(), NULL);
            if (msgNotOk) throw std::runtime_error("Error during display of game's new record message box.");

            // Update player record score.
            this->maxScorePlayer = this->GetScore();
          }
        }

        // Ask if the player wants to play a new game round
        Game::ConfirmationBoxButtonId selection = this->ShowConfirmationBox(
          "Play again?",
          "Error during the display of \"play again?\" question window.",
          true // 'Yes' as default option
        );

        if (selection == Game::ConfirmationBoxButtonId::Yes) {
          // Start a new game round.
          this->NewRound();
        } else {
          // End the game
          this->running = false;
        }
      }
    }
  }

  // Stores the game state in a save file, for it to be resumed in the next execution.
  StoreSaveFile();
}

void Game::UpdateState(const Controller::UserCommand command) {
  // First, process the input user command.
  if (command == Controller::UserCommand::Quit) {
    // End the game
    this->running = false;
  } else if (command == Controller::UserCommand::ToggleFpsCtrl) {
    // Only enable speed mode switching if snake is in Auto mode.
    if (snake.IsAutoModeOn()) this->fpsCtrlActv = !fpsCtrlActv;
  } else if (command == Controller::UserCommand::Pause) {
    this->paused = !paused;
  } else if (command == Controller::UserCommand::EraseData) {
    // Display a message box asking for confirmation on the intended action.
    Game::ConfirmationBoxButtonId selection = this->ShowConfirmationBox(
      "Do you really want to proceed with erasing user data?\nPlease note: all game records,"
        " AI learning and current round state will be lost forever, and a new game round will begin.",
      "Error during display of game's data-erasing confirmation message box."
    );

    if (selection == Game::ConfirmationBoxButtonId::Yes) {
      // Restart game data and AI learning.
      this->ResetData();

      // Display success message in erasing user game data.
      std::string message = "User data was successfully erased!\nA new game round will now start.";
      int msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "The Snake Game", message.c_str(), NULL);
      if (msgNotOk) throw std::runtime_error("Error during display of game's data-erasing success message box.");

    } else {
      // No user data is erased, and game is resumed.
      std::string message = "No user data was erased.\nCurrent game round will now be resumed.";
      int msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "The Snake Game", message.c_str(), NULL);
      if (msgNotOk) throw std::runtime_error("Error during display of game's data-erasing cancel message box.");

    }

  } else if (command == Controller::UserCommand::ToggleAutoMode) {
    // Upon switching from auto to manual mode or vice versa, tell user that game round
    // would be reset and current round data would be lost, and ask for confirmation.
    std::string question;
    if (snake.IsAutoModeOn()) {
      question = "If you switch to manual (player) mode, a new game round will start and current"
        " round data/score will be lost.\nDo you want to proceed?";
    } else {
      question = "If you switch to auto (AI) mode, a new game round will start and current"
        " round data/score will be lost.\nDo you want to proceed?";
    }
    Game::ConfirmationBoxButtonId selection = this->ShowConfirmationBox(
      question,
      "Error during display of game's control mode switching confirmation box."
    );

    if (selection == Game::ConfirmationBoxButtonId::Yes) {
      // If the user confirms, pass the command to be processed by the snake.
      snake.ProcessUserCommand(command);

      // Display mode switch message.
      std::string message;
      if (snake.IsAutoModeOn()) message = "Control mode was switched to auto (AI) mode.\nA new game round will now start.";
      else {
        message = "Control mode was switched to manual (player) mode.\nA new game round will now start.";
        
        // When switching to Manual mode, make sure Speed control is enabled, otherwise game will be unplayable.
        this->fpsCtrlActv = true;
      }
      int msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "The Snake Game", message.c_str(), NULL);
      if (msgNotOk) throw std::runtime_error("Error during display of game's control mode switch message box.");

      // Reset game round.
      this->NewRound();

    } else {
      // No mode switch, and game is resumed.
      std::string message;
      if (snake.IsAutoModeOn()) message = "Control was kept at auto (AI) mode.\nCurrent game round will now be resumed.";
      else message = "Control was kept at manual (player) mode.\nCurrent game round will now be resumed.";
      int msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "The Snake Game", message.c_str(), NULL);
      if (msgNotOk) throw std::runtime_error("Error during display of game's control mode maintainance message box.");

    }

  } else {
    // For any other command, just pass the command on to be processed by the snake object.
    snake.ProcessUserCommand(command);
  }

  // Next, update the game state accordingly.
  // If the snake is deceased or game is paused, no world update needs to be done.
  if (!snake.IsAlive() || paused) return;

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

        // Everytime the snake eats, if in automode, empty the covered grid positions.
        if (snake.IsAutoModeOn()) coveredPositions.clear();

        // Now that the food has been eaten, make new food appear in a free grid tile.
        if (!world.GrowFood()) {
          // If a new food cannot be placed, the game has been won.
          this->victory = true;
        }
        
      } else {
        // If the snake hasn't collided or eaten, just move it to the new tile.
        snake.SetEvent(Snake::Event::NewTile);

        // If in automode, the new tile gets looked up for in the covered position+direction container.
        if (snake.IsAutoModeOn()) {
          auto searchResult = coveredPositions.find(&world.GetElementRef(targetHeadPosition));
          if (searchResult != coveredPositions.end()) {
            // If position is present in the covered positions list, check if direction is also the same as current.
            if (searchResult->second == snake.GetDirection()) {
              // If the direction from which the position was entered is the same as current one, kill the snake
              // and end current game round to prevent an endless game loop.
              snake.SetEvent(Snake::Event::Killed);
            } else {
              // Otherwise, update covered position in container with a new mapped value of current direction.
              coveredPositions[&world.GetElementRef(targetHeadPosition)] = snake.GetDirection();
            }
          } else {
            // If the position isn't present yet in the container, add it.
            coveredPositions[&world.GetElementRef(targetHeadPosition)] = snake.GetDirection();
          }
        }
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

void Game::NewRound() {
  // Reinitialize the world.
  world.Init();

  // Reinitialize the snake.
  snake.Init();

  // Empty the covered positions container.
  coveredPositions.clear();

  // Reset the victory state.
  this->victory = false;
}

void Game::ResetData() {
  // Restart game data and AI learning.
  // Start by deleting "save_state.txt" file, if it exists.
  remove(SAVE_STATE_FILE_PATH);

  // Next, reset max player and AI scores.
  this->maxScorePlayer = 0;
  this->maxScoreAI = 0;

  // Resets the snake's MLP parameters to the dafault ones.
  snake.ResetMLP();

  // Reset the snake's Genetic Algorithm state, and parameters to the default ones.
  snake.ResetGenAlg();

  // Start a new game round.
  this->NewRound();
}

void Game::StoreSaveFile() const {
  // Remove previous save file, in case it exists.
  remove(SAVE_STATE_FILE_PATH);

  // Create the file and open.
  std::ofstream file(SAVE_STATE_FILE_PATH);

  if (file.is_open()) {
    // Store the game state.
    file << maxScorePlayer << std::endl;
    file << maxScoreAI << std::endl;

    // Save the mlp configuration and genetic algorithm state.
    snake.StoreState(file);
  } else throw std::runtime_error("Couldn't write game history state to \"../save_state.txt\" file.");

  file.close();
}

void Game::LoadSaveFile() {
  std::ifstream file(SAVE_STATE_FILE_PATH);
  if (file.is_open()) {
    // Restore the game state.
    file >> maxScorePlayer;
    file >> maxScoreAI;

    // Restore the mlp configuration and genetic algorithm state.
    snake.LoadState(file);
  }
  file.close();
}

Game::ConfirmationBoxButtonId Game::ShowConfirmationBox(const std::string& message, const std::string& errMessage,
    const bool yesDefaultOption) const {
  SDL_MessageBoxButtonData buttons[] = { 
    /* .flags, .buttonid, .text */
    { 0, (int) Game::ConfirmationBoxButtonId::Cancel, "No/Cancel" },
    { 0, (int) Game::ConfirmationBoxButtonId::Yes, "Yes" }
  };

  if (yesDefaultOption) {
    buttons[1].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
  } else {
    buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT | SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
  }

  const SDL_MessageBoxColorScheme colorScheme = {
    { /* .colors (.r, .g, .b) */
      /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
      { 255,   0,   0 },
      /* [SDL_MESSAGEBOX_COLOR_TEXT] */
      {   0, 255,   0 },
      /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
      { 255, 255,   0 },
      /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
      {   0,   0, 255 },
      /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
      { 255,   0, 255 }
    }
  };

  const SDL_MessageBoxData confirmationBox = {
    SDL_MESSAGEBOX_INFORMATION, /* .flags */
    NULL, /* .window */
    "The Snake Game", /* .title */
    message.c_str(), /* .message */
    SDL_arraysize(buttons), /* .numbuttons */
    buttons, /* .buttons */
    &colorScheme /* .colorScheme */
  };

  int selection = (int) Game::ConfirmationBoxButtonId::None;
  if (SDL_ShowMessageBox(&confirmationBox, &selection)) {
    throw std::runtime_error(errMessage);
  }

  return (Game::ConfirmationBoxButtonId) selection;
}