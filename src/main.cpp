#include <iostream>
#include "controller.h"
#include "game.h"
#include "renderer.h"
#include <tensorflow/c/c_api.h>

int main() {
  constexpr std::size_t kFramesPerSecond{60};
  constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
  constexpr std::size_t kScreenWidth{620};
  constexpr std::size_t kScreenHeight{620};
  constexpr std::size_t kGridWidth{31};
  constexpr std::size_t kGridHeight{31};

  // Test call to Tensorflow API
  printf("This project uses TensorFlow C library version %s\n", TF_Version());

  Renderer renderer(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight);
  Controller controller;
  Game game(kGridWidth, kGridHeight);
  game.Run(controller, renderer, kMsPerFrame);
  std::cout << "Game has terminated successfully!\n";
  std::cout << "Score: " << game.GetScore() << "\n";
  std::cout << "Size: " << game.GetSize() << "\n";
  return 0;
}