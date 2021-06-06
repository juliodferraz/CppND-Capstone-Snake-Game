#include <iostream>

#include "controller.h"
#include "game.h"
#include "renderer.h"
#include "build.h"

#include <tensorflow/c/c_api.h>
//#include "tensorflow/cc/cc_api.h"

int main(int argc, char **argv) {
  constexpr std::size_t kFramesPerSecond{60};
  constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
  constexpr std::size_t kScreenWidth{620};
  constexpr std::size_t kScreenHeight{620};
  constexpr std::size_t kGridSideSize{31};

  Renderer renderer(kScreenWidth, kScreenHeight, kGridSideSize);
  Controller controller;
  Game game(kGridSideSize);

  game.Run(controller, renderer, kMsPerFrame);

  std::cout << "Game has terminated successfully!" << std::endl;
  std::cout << "Score: " << game.GetScore() << std::endl;

  return 0;
}