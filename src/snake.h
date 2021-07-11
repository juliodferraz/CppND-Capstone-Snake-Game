#ifndef SNAKE_H
#define SNAKE_H

#include <deque>
#include <random>

#include "controller.h"
#include "matrix.h"
#include "build.h"
#include "coords2D.h"

class Snake {
 public:
  /**
   *  \brief Snake direction enum. Values are clockwise ordered.
   */
  enum class Direction { Up, Right, Down, Left };

  /**
   *  \brief Snake event enum, representing the possible results of a movement.
   */
  enum class Event { SameTile, NewTile, Ate, Collided };

  /**
   *  \brief Snake action enum, representing the possible decisions of the snake AI model (i.e. move either forward, left or right of the current direction).
   */
  enum class Action { MoveFwd, MoveLeft, MoveRight };

  // TODO: comment
  Snake(const SDL_Point& startPosition);

  /**
   *  \brief Initializes the snake's parameters and world view.
   */
  void Init(const SDL_Point& startPosition);

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
  std::size_t GetSize() const { return positionQueue.size(); }

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
   *  \brief Returns a const reference to the queue holding the current snake position.
   *  \return Const reference to current snake position queue.
   */
  const std::deque<SDL_Point>& GetPositionQueue() const { return positionQueue; }

  /**
   *  \brief Returns the position of the snake's tail. In case the snake's size is 1, returns the head position.
   *  \return The coordinates of the snake's tail in the world grid (i.e. from player's perspective).
   */
  SDL_Point GetTailPosition() const { return positionQueue.back(); }
  SDL_Point GetHeadPosition() const { return positionQueue.front(); }
  SDL_Point GetTargetHeadPosition() const { return targetHead; }

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
   *  \brief Calculates the snake's AI model decision for the next snake action, based on the world state.
   */
  void DefineAction();

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

  /**
   *  \brief The current snake direction.
   */
  Direction direction{Direction::Up};

  /**
   *  \brief Double-ended queue containing the snake's head and body parts coordinates in the world. The double-ended queue
   * has constant complexity for push and pop operations at both queue ends, which makes it more efficient to be used here
   * instead of a vector (which displays linear complexity for operations at its front).
   */
  std::deque<SDL_Point> positionQueue;
  Coords2D targetHead;

  /**
   *  \brief Indicates snake's current life state (alive or deceased).
   */
  bool alive{true};

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

  /**
   *  \brief Random number generator defining the snake direction changes during auto mode.
   */
  std::default_random_engine generator;

  /**
   *  \brief Uniform real distribution to be used during calculation of snake direction changes during auto mode.
   */
  std::uniform_real_distribution<float> random_direction_distribution{0.0, 1.0};
};

#endif