#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include "SDL.h"

#define INITIAL_SNAKE_SPEED 0.1f

class Snake {
 public:
  /**
   *  \brief Snake direction enum. Values are clockwise ordered.
   */
  enum class Direction { kUp, kRight, kDown, kLeft };

  Snake(int grid_width, int grid_height)
      : grid_width(grid_width),
        grid_height(grid_height),
        head_x(grid_width / 2),
        head_y(grid_height / 2) {}

  void Update();

  void GrowBody();
  bool SnakeCell(int x, int y);

  /**
   *  \brief Makes the snake alive again, with its initial size, effectively resetting the game. If the snake is already alive, nothing is done.
   */
  void Resurrect();

  /**
   *  \brief Toggles the snake mode between auto and manual (controllable by the player).
   */
  void ToggleAutoMode();

  /**
   *  \brief Indicates if auto mode is on.
   *  \return True, if the snake is autonomous. False, if it's controllable by the player.
   */
  bool IsAutoModeOn() { return automode; }

  Direction direction = Direction::kUp;

  float speed{INITIAL_SNAKE_SPEED};
  int size{1};
  bool alive{true};
  float head_x;
  float head_y;
  std::vector<SDL_Point> body;

 private:
  void UpdateHead();
  void UpdateBody(SDL_Point &current_cell, SDL_Point &prev_cell);

  bool growing{false};
  int grid_width;
  int grid_height;

  /**
   *  \brief True, if the snake is autonomous. False, if it's controllable by the player.
   */
  bool automode{true};
};

#endif