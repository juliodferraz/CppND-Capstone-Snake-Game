#ifndef SNAKE_H
#define SNAKE_H

#include <vector>

#include "controller.h"
#include "matrix.h"
#include "build.h"

#include "SDL.h"

class Snake {
 public:
  /**
   *  \brief Snake direction enum. Values are clockwise ordered.
   */
  enum class Direction { Up = 0, Right = 1, Down = 2, Left = 3};

  /**
   *  \brief Snake event enum, representing the possible results of a movement.
   */
  enum class Event { SameTile, NewTile, Ate, Collided };

  /**
   *  \brief Snake action enum, representing the possible decisions of the snake AI model (i.e. move either forward, left or right of the current direction).
   */
  enum class Action { MoveFwd, MoveLeft, MoveRight };

  /**
   *  \brief Struct type holding the current location of the snake head its body.
   */
  struct Position {
    /**
    *  \brief The global discrete location of the snake's head on the grid.
    */
    SDL_Point head;

    /**
    *  \brief The global location of the snake body parts.
    */
    std::vector<SDL_Point> body;
  };

  // TODO: comment
  Snake(const int& grid_side_size);

  /**
   *  \brief Initializes the snake's parameters and world view.
   */
  void Init();

  /**
   *  \brief Updates the snake internal state based on the user command.
   *  \param command Latest command issued by the player.
   */
  void ProcessUserCommand(const Controller::UserCommand& command);

  /**
   *  \brief Moves the snake (following its current direction) and updates its location in the world.
   */
  void Move();

  /**
   *  \brief Returns the current snake size.
   *  \return Current snake size.
   */
  std::size_t GetSize() const { return size; }
  int GetHungerLevel() const { return hungerLevel; }
  void ClearHunger() { this->hungerLevel = 0; }

  /**
   *  \brief Returns the current snake action its AI model decided for.
   *  \return Current snake action.
   */
  Action GetAction() const { return action; }

  /**
   *  \brief Returns the latest snake event.
   *  \return Latest snake event.
   */
  Event GetEvent() const { return event; }

  /**
   *  \brief Returns the current snake direction.
   *  \return Latest snake event.
   */
  Direction GetDirection() const { return direction; }

  /**
   *  \brief Sets the latest snake event, resulting from its last action, and updates other internal parameters based on the event.
   *  \param event The event to be set.
   */
  void SetEvent(const Event& event);

  // TODO: comment
  bool SetDirection(const Direction& direction);

  /**
   *  \brief Indicates if auto mode is on.
   *  \return True, if the snake is autonomous. False, if it's controllable by the player.
   */
  bool IsAutoModeOn() const { return automode; }

  /**
   *  \brief Indicates if snake is alive.
   *  \return True, if the snake is alive. False, if it is deceased.
   */
  bool IsAlive() const { return alive; }

  /**
   *  \brief Returns a const reference to the struct holding the current snake position.
   *  \return Reference to current snake position struct.
   */
  const struct Position& GetPosition() const { return position; }

  /**
   *  \brief Returns the position of the snake's tail. In case the snake's size is 1, returns the head position.
   *  \return Position of the snake's tail in the world grid (i.e. from player's perspective).
   */
  SDL_Point GetTailPosition() const { return (size > 1)? position.body.front() : position.head; }

  /**
   *  \brief Returns the direction located left (relatively) of the input direction.
   *  \param reference Reference direction.
   *  \return Direction located left of the input one.
   */
  static Direction GetLeftOf(const Direction& reference);

  /**
   *  \brief Returns the direction located right (relatively) of the input direction.
   *  \param reference Reference direction.
   *  \return Direction located right of the input one.
   */
  static Direction GetRightOf(const Direction& reference);

  /**
   *  \brief Returns the direction contrary to the input direction.
   *  \param reference Reference direction.
   *  \return Direction opposite to the input one.
   */
  static Direction GetOppositeOf(const Direction& reference);

  /**
   *  \brief Advances the snake world view in one tile ahead (considering the current snake direction) and updates its body location.
   *  \param prev_head_position Previous head position in the world, for the snake body update.
   */
  void UpdateBody(const SDL_Point& prev_head_position);

 private:
  /**
   *  \brief Toggles the snake mode between auto and manual (controllable by the player).
   */
  inline void ToggleAutoMode() { automode = !automode; }

  /**
   *  \brief Makes the snake act.
   *  \param input Target action.
   */
  void Act(const Action& input);

  int grid_side_size;
  float head_x;
  float head_y;

  /**
   *  \brief The current snake direction.
   */
  Direction direction{Direction::Up};

  /**
   *  \brief The snake's position in the world.
   */
  Position position;

  /**
   *  \brief Indicates snake's current life state (alive or deceased).
   */
  bool alive{true};

  /**
   *  \brief The snake size (begins at 1, for only the head exists initially).
   */
  std::size_t size{1};

  /**
   *  \brief Current snake speed.
   */
  const float speed{0.2f};

  /**
   *  \brief The latest snake action.
   */
  Action action{Action::MoveFwd};

  /**
   *  \brief The latest snake event, as a result of its action.
   */
  Event event{Event::SameTile};

  /**
   *  \brief True, if the snake is autonomous. False, if it's controllable by the player.
   */
  bool automode{true};

  int hungerLevel{0};
};

#endif