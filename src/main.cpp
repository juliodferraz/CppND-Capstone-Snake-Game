#include <iostream>
#include <stdexcept>

#include "game.h"
#include "config.h"

#include "SDL.h"

int main(int argc, char **argv) {
  try {
    std::string message = "The Snake Game will begin!\n"
      "Please find the game controls summary below:\n"
      "- Arrow keys: controls the snake in Manual mode (i.e. player in control);\n"
      "- 'A' key: toggles Auto mode (i.e. AI in control) on or off;\n"
      "- 'S' key: toggles Speed mode (i.e. no frame rate control) on or off (obs.: only enabled in Auto mode);\n"
      "- 'P' key: pauses the game (or resumes it);\n"
      "- 'E' key: erases and resets all user data, inclusing game history and AI learning;\n"
      "- Close game window: ends the game and exit.";
    int msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "The Snake Game", message.c_str(), NULL);
    if (msgNotOk) throw std::runtime_error("Error during display of game's starting message box.");

    Game game(WINDOW_WIDTH, WINDOW_HEIGHT, GRID_SIDE_LENGTH);
    game.Run(FRAME_PERIOD_MS, TIME_LIMIT_F);

    message = "Game has terminated successfully!\n"
      "Player Max Score: " + std::to_string(game.GetMaxScorePlayer()) + "\n"
      "AI Max Score: " + std::to_string(game.GetMaxScoreAI()) + "\n";
    msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "The Snake Game", message.c_str(), NULL);
    if (msgNotOk) throw std::runtime_error("Error during display of game's ending message box.");

  } catch(const std::exception& e) {
    std::string message{"An error occurred. Please try restarting the game.\nError: " + std::string(e.what())};
    int msgNotOk = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "The Snake Game", message.c_str(), NULL);
    if (msgNotOk) {
      // In case message box returned non-zero error code, output exception message to error stream.
      std::cerr << "An error occurred. Please try restarting the game.\nError: " << e.what() << std::endl;
    }
    
    return -1;
  }

  return 0;
}