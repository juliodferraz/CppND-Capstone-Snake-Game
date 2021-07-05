#include "renderer.h"
#include <iostream>
#include <string>

Renderer::Renderer(const std::size_t screen_width,
                   const std::size_t screen_height,
                   const std::size_t grid_side_size)
    : screen_width(screen_width),
      screen_height(screen_height),
      grid_side_size(grid_side_size) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create Window
  sdl_window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, screen_width,
                                screen_height, SDL_WINDOW_SHOWN);

  if (nullptr == sdl_window) {
    std::cerr << "Window could not be created.\n";
    std::cerr << " SDL_Error: " << SDL_GetError() << "\n";
  }

  // Create renderer
  sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
  if (nullptr == sdl_renderer) {
    std::cerr << "Renderer could not be created.\n";
    std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
  }

  #if DEBUG_MODE
    // TODO: organize this
    // Create snake view Window
    screen_width_snake = screen_width / 2;
    screen_height_snake = screen_height / 2;
    sdl_window_snake = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, screen_width_snake,
                                  screen_height_snake, SDL_WINDOW_SHOWN);

    if (nullptr == sdl_window_snake) {
      std::cerr << "Window could not be created.\n";
      std::cerr << " SDL_Error: " << SDL_GetError() << "\n";
    }

    // Create renderer
    sdl_renderer_snake = SDL_CreateRenderer(sdl_window_snake, -1, SDL_RENDERER_ACCELERATED);
    if (nullptr == sdl_renderer_snake) {
      std::cerr << "Renderer could not be created.\n";
      std::cerr << "SDL_Error: " << SDL_GetError() << "\n";
    }
  #endif
}

Renderer::~Renderer() {
  SDL_DestroyWindow(sdl_window);
  #if DEBUG_MODE
    SDL_DestroyWindow(sdl_window_snake);
  #endif
  SDL_Quit();
}

void Renderer::Render(const World& world) {
  const Snake& snake = world.GetSnake();
  const std::deque<Coords2D>& snake_position = snake.GetPosition();
  const SDL_Point& food_position = world.GetFoodPosition();

  SDL_Rect block;
  block.w = screen_width / grid_side_size;
  block.h = screen_height / grid_side_size;

  // Clear screen
  SDL_SetRenderDrawColor(sdl_renderer, 0x1E, 0x1E, 0x1E, 0xFF);
  SDL_RenderClear(sdl_renderer);

  // Render food
  SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xCC, 0x00, 0xFF);
  block.x = food_position.x * block.w;
  block.y = food_position.y * block.h;
  SDL_RenderFillRect(sdl_renderer, &block);

  // Render snake's body
  SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  for (Coords2D const &point : snake_position) {
    block.x = point.GetIntX() * block.w;
    block.y = point.GetIntY() * block.h;
    SDL_RenderFillRect(sdl_renderer, &block);
  }

  // Render snake's head
  block.x = snake_position.front().GetIntX() * block.w;
  block.y = snake_position.front().GetIntY() * block.h;
  if (snake.IsAlive()) {
    SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x7A, 0xCC, 0xFF);
  } else {
    SDL_SetRenderDrawColor(sdl_renderer, 0xFF, 0x00, 0x00, 0xFF);
  }
  SDL_RenderFillRect(sdl_renderer, &block);

  // Update Screen
  SDL_RenderPresent(sdl_renderer);

  #if DEBUG_MODE
    // Snake world view screen rendering
    const Matrix& view = snake.GetWorldView();
    block.w = screen_width_snake / grid_side_size;
    block.h = screen_height_snake / grid_side_size;

    // Clear screen
    SDL_SetRenderDrawColor(sdl_renderer_snake, 0x1E, 0x1E, 0x1E, 0xFF);
    SDL_RenderClear(sdl_renderer_snake);

    // Draw all elements
    block.y = 0;
    for(int y = 0; y < grid_side_size; y++) {
      block.x = 0;
      for(int x = 0; x < grid_side_size; x++) {
        switch(snake.GetWorldViewElement({x,y})) {
          case Snake::WorldElement::None:
            // Do nothing
            break;
          case Snake::WorldElement::Body:
            // Render body
            SDL_SetRenderDrawColor(sdl_renderer_snake, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderFillRect(sdl_renderer_snake, &block);
            break;
          case Snake::WorldElement::Head:
            // Render head
            if (snake.IsAlive()) {
              SDL_SetRenderDrawColor(sdl_renderer_snake, 0x00, 0x7A, 0xCC, 0xFF);
            } else {
              SDL_SetRenderDrawColor(sdl_renderer_snake, 0xFF, 0x00, 0x00, 0xFF);
            }
            SDL_RenderFillRect(sdl_renderer_snake, &block);
            break;
          default:
            // Render food
            SDL_SetRenderDrawColor(sdl_renderer_snake, 0xFF, 0xCC, 0x00, 0xFF);
            SDL_RenderFillRect(sdl_renderer_snake, &block);
            break;
        }
        block.x += block.w;
      }
      block.y += block.h;
    }

    // Update Screen
    SDL_RenderPresent(sdl_renderer_snake);
  #endif
}

void Renderer::UpdateWindowTitle(int score, int fps, bool automode) {
  std::string title{"Snake Score: " + std::to_string(score) + " FPS: " + std::to_string(fps)};
  if (automode) title += " (auto mode)";
  else title += " (manual mode)";
  SDL_SetWindowTitle(sdl_window, title.c_str());
}
