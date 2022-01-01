#include <iostream>
#include <stdexcept>

#include "game.h"
#include "config.h"

int main(int argc, char **argv) {
  try {
    Game game(WINDOW_WIDTH, WINDOW_HEIGHT, GRID_SIDE_LENGTH);
    game.Run(FRAME_PERIOD_MS, TIME_LIMIT_F);

    std::cout << "Game has terminated successfully!" << std::endl;
    std::cout << "Max Score: " << game.GetMaxScore() << std::endl;

  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  return 0;
}