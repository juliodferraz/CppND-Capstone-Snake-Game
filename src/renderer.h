#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "SDL.h"
#include "world.h"
#include "build.h"

class Renderer {
 public:
  Renderer(const std::size_t screen_width, const std::size_t screen_height,
           const std::size_t grid_side_size);
  ~Renderer();

  void Render(const World& world);

  /**
   *  \brief Updates information in game window title.
   *  \param score Current player score.
   *  \param fps Current frames per second value.
   *  \param automode Flag indicating if game is in auto mode.
   */
  void UpdateWindowTitle(int score, int fps, bool automode);

 private:
  SDL_Window *sdl_window;
  SDL_Renderer *sdl_renderer;

  const std::size_t screen_width;
  const std::size_t screen_height;
  const std::size_t grid_side_size;

#if DEBUG_MODE
  // TODO: organize this
  SDL_Window *sdl_window_snake;
  SDL_Renderer *sdl_renderer_snake;

  std::size_t screen_width_snake;
  std::size_t screen_height_snake;
#endif
};

#endif