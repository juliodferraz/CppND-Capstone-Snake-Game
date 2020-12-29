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

  /**
   *  \brief Snake event enum, representing the possible results of a change of direction.
   */
  enum class Event { Moved, Ate, Collided };

  /**
   *  \brief Snake action enum, representing the possible decisions of the snake AI model (i.e. move either forward, left or right of the current direction).
   */
  enum class Action { MoveFwd, MoveLeft, MoveRight };

  Snake(int grid_width, int grid_height)
      : grid_width(grid_width),
        grid_height(grid_height),
        head_x(grid_width / 2),
        head_y(grid_height / 2),
        head{static_cast<int>(head_x), static_cast<int>(head_y)},
        vision{{(grid_width - 1) / 2, grid_height - 1}, {0,0}, std::vector<SDL_Point>{}} {}

  void Update();

  bool SnakeCell(int x, int y);

  /**
   *  \brief Updates the snake's speed and its internal state, to indicate it has eaten and is growing.
   */
  void Eat();

  /**
   *  \brief Updates the snake's AI model based on the latest event (i.e. the result of its actions).
   */
  void Learn();

  /**
   *  \brief Updates the current food and body sections location in the snake's vision.
   *  \param food Current food location in the grid, from the player's perspective.
   */
  void SeeWorld(const SDL_Point& food);

  /**
   *  \brief Calculates the snake's AI model decision for the next snake action, based on the world state.
   */
  void DefineAction();

  /**
   *  \brief Returns the current snake action its AI model decided for.
   *  \return Current snake action.
   */
  Action GetAction() { return action; }

  /**
   *  \brief Sets current snake action equal to a specific value (normally coming from user input, during manual mode).
   *  \param input Input action.
   */
  void SetAction(Action&& input) { action = input; }

  /**
   *  \brief Returns the latest snake event.
   *  \return Latest snake event.
   */
  Event GetEvent() { return event; }

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

  /**
   *  \brief Indicates if snake is alive.
   *  \return True, if the snake is alive. False, if it is deceased.
   */
  bool IsAlive() { return alive; }

  /**
   *  \brief Returns the direction located left (relatively) of the input direction.
   *  \param reference Reference direction.
   *  \return Direction located left of the input one.
   */
  static Direction GetLeftOfDirection(const Direction& reference);

  /**
   *  \brief Returns the direction located right (relatively) of the input direction.
   *  \param reference Reference direction.
   *  \return Direction located right of the input one.
   */
  static Direction GetRightOfDirection(const Direction& reference);

  /**
   *  \brief The current snake direction.
   */
  Direction direction = Direction::kUp;

  int size{1};
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

  int grid_width;
  int grid_height;

  /**
   *  \brief Indicates snake's current life state (alive or deceased).
   */
  bool alive{true};

  /**
   *  \brief Flag used to request snake's growth during its body update. If True, it is reset to False as soon as the snake's body gets incremented.
   */
  bool growing{false};

  /**
   *  \brief Current snake speed.
   */
  float speed{INITIAL_SNAKE_SPEED};

  /**
   *  \brief The latest snake event.
   */
  Event event = Event::Moved;

  /**
   *  \brief The latest snake action.
   */
  Action action = Action::MoveFwd;

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