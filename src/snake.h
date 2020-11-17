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
        head_y(grid_height / 2),
        head{static_cast<int>(head_x), static_cast<int>(head_y)},
        vision{{(grid_width - 1) / 2, grid_height - 1}, {0,0}, std::vector<SDL_Point>{}} {}

  void Update();

  void GrowBody();
  bool SnakeCell(int x, int y);

  /**
   *  \brief Updates the current food and body sections location in the snake's vision.
   *  \param food Current food location in the grid, from the player's perspective.
   */
  void SeeWorld(const SDL_Point& food);

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

  /**
   *  \brief The current discrete location of the snake's head on the grid.
   */
  SDL_Point head;

  std::vector<SDL_Point> body;

 private:
  void UpdateHead();
  void UpdateBody(const SDL_Point &prev_head_cell);

  /**
   *  \brief Returns the position of a given world point from the snake's perspective, relative to its head.
   *  \param point A position in the game screen, from the player's viewpoint.
   *  \return The equivalent point position from the snake's perspective.
   */
  SDL_Point ToSnakeVision(const SDL_Point& point);

  bool growing{false};
  int grid_width;
  int grid_height;

  /**
   *  \brief True, if the snake is autonomous. False, if it's controllable by the player.
   */
  bool automode{true};

  /**
   *  \brief Struct type representing a snake's perception of the world around it.
   */
  struct Vision {
    /**
     *  \brief Location of the snake's head used as base reference for its world view. From the snake's perspective, its head is on the middle-bottom of the vision grid, allowing it to see what lies ahead and by its sides.
     */
    const SDL_Point head;

    /**
     *  \brief Location of the food relative to the snake's head and current direction.
     */
    SDL_Point food;

    /**
     *  \brief Location of the body sections relative to the snake's head and current direction.
     */
    std::vector<SDL_Point> body;
  };

  /**
   *  \brief The snake's perception of the world around it.
   */
  struct Vision vision;
};

#endif