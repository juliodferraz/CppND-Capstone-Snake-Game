#include "renderer.h"
#include <iostream>
#include <string>
#include <stdexcept>

Renderer::Renderer(const unsigned int winWidth,
                   const unsigned int winHeight,
                   const unsigned int gridSideLen)
    : winWidth(winWidth),
      winHeight(winHeight),
      gridSideLen(gridSideLen) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";

    throw std::runtime_error("SDL lib could not initialize.");
  }

  // Create Window
  sdlWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, winWidth,
                                winHeight, SDL_WINDOW_SHOWN);
  if (sdlWindow == nullptr) {
    std::cerr << "Window could not be created.\n";
    std::cerr << " SDL_Error: " << SDL_GetError() << "\n";

    // Terminate SDL objects that had been created.
    SDL_Quit();

    throw std::runtime_error("SDL window could not be created.");
  }

  // Create renderer
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
  if (sdlRenderer == nullptr) {
    std::cerr << "Renderer could not be created.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";

    // Terminate window and SDL objects that had been created.
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    throw std::runtime_error("SDL renderer could not be created.");
  }
}

Renderer::~Renderer() {
  SDL_DestroyRenderer(sdlRenderer);
  SDL_DestroyWindow(sdlWindow);
  SDL_Quit();
}

void Renderer::Render(const World& world, const Snake& snake) {
  // Clear screen
  SDL_SetRenderDrawColor(sdlRenderer, 0x1E, 0x1E, 0x1E, 0xFF);
  SDL_RenderClear(sdlRenderer);

  // Initialize the width and height of each screen grid unit.
  SDL_Rect block;
  block.w = winWidth / gridSideLen;
  block.h = winHeight / gridSideLen;

  // Starting from position (0,0), cover all grid positions, 
  // printing to the screen according to the element in each
  // position.
  block.x = 0;
  for (int i = 0; i < gridSideLen; i++) {
    block.y = 0;
    for (int j = 0; j < gridSideLen; j++) {
      switch (world.GetElement({i,j})) {
        case World::Element::Wall:
          SDL_SetRenderDrawColor(sdlRenderer, 0x80, 0x80, 0x80, 0xFF);
          SDL_RenderFillRect(sdlRenderer, &block);
          break;
        case World::Element::SnakeBody:
          SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
          SDL_RenderFillRect(sdlRenderer, &block);
          break;
        case World::Element::SnakeHead:
          if (snake.IsAlive()) {
            SDL_SetRenderDrawColor(sdlRenderer, 0x00, 0x7A, 0xCC, 0xFF);
          } else {
            SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0x00, 0x00, 0xFF);
          }
          SDL_RenderFillRect(sdlRenderer, &block);
          break;
        case World::Element::Food:
          SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0xCC, 0x00, 0xFF);
          SDL_RenderFillRect(sdlRenderer, &block);
          break;
        default: //case World::Element::None
          // Do nothing
          break;
      }
      block.y += block.h;
    }
    block.x += block.w;
  }

  // Update Screen
  SDL_RenderPresent(sdlRenderer);
}

void Renderer::UpdateWindowTitle(const unsigned int score, const unsigned int fps, const unsigned int maxScorePlayer, 
    const bool automode, const unsigned int maxScoreAI, const unsigned int genCnt, const unsigned int indCnt, 
    const bool gamePaused) {
  std::string title{"FPS: " + std::to_string(fps)};

  if (automode) {
    title += " / Auto";
    title += ", Score: " + std::to_string(score);
    title += ", Record: " + std::to_string(maxScoreAI);

    if (gamePaused) {
      title += " / Paused";
    } else {
      title += " / Learning...";
      title += " Gen: " + std::to_string(genCnt);
      title += ", Ind: " + std::to_string(indCnt);
    }

  } else {
    title += " / Manual";
    title += ", Score: " + std::to_string(score);
    title += ", Record: " + std::to_string(maxScorePlayer);
    if (gamePaused) {
      title += " / Paused";
    }
  }

  SDL_SetWindowTitle(sdlWindow, title.c_str());
}
