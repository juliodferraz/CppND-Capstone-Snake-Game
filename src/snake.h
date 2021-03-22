#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include <random>

#include "controller.h"
#include "matrix.h"
#include "build.h"

#include "SDL.h"
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/lib/gtl/array_slice.h"

using namespace tensorflow;
using namespace tensorflow::ops;

#define INITIAL_SNAKE_SPEED 0.1f

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

  /**
   *  \brief Enum type representing the possible contents of a tile in the snake world view grid.
   */
  enum class WorldElement { None = 0, Head = -1, Body = -2, Food = 2};

  /**
   *  \brief Struct type holding the current location of the snake head its body.
   */
  struct Position {
    /**
    *  \brief The current discrete location of the snake's head on the grid.
    */
    SDL_Point head;

    /**
    *  \brief The location of the snake body parts.
    */
    std::vector<SDL_Point> body;
  };

  // TODO: comment
  Snake(const int& grid_side_size);

  /**
   *  \brief Initializes the snake's parameters and world view.
   *  \param food_position The food position in the world grid.
   */
  void Init(const SDL_Point& food_position);

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

  /**
   *  \brief Returns the current snake action its AI model decided for.
   *  \return Current snake action.
   */
  Action GetAction() const { return action; }

  /**
   *  \brief Returns the latest snake event, resulting from its last action.
   *  \return Latest snake event.
   */
  Event GetEvent() const { return event; }

  /**
   *  \brief Updates the element located in a specific snake world view position.
   *  \param position The target position, from world grid perspective (i.e. player's perspective).
   *  \param new_element The new element to be set.
   */
  void SetWorldViewElement(const SDL_Point& position, const WorldElement& new_element);

  // TODO: comment
  const Matrix& GetWorldView() const { return vision.world; }

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
  SDL_Point GetTailPosition() const { return (size > 1)? position.body.back() : position.head; }

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
   *  \brief Returns the element present in the input position, in the snake world view.
   *  \param position A position in the snake world view grid.
   *  \return Element present in the position.
   */
  WorldElement GetWorldViewElement(const SDL_Point& position) const {
    return static_cast<Snake::WorldElement>(vision.world.GetAt(position.y, position.x));
  }

 private:
  /**
   *  \brief Moves the snake head according to its current direction and speed.
   */
  void MoveHead();

  /**
   *  \brief Updates the snake's internal state, based on input event (e.g. eating or collision).
   *  \param input The event that occurred and needs to be reflected in the snake state.
   */
  void ProcessEvent(const Snake::Event& input);

  /**
   *  \brief Senses the tile the snake is about to enter, raising any resulting event (e.g. eating or collision).
   */
  void SenseFrontTile();

  /**
   *  \brief Advances the snake world view in one tile ahead (considering the current snake direction) and updates its body location.
   *  \param prev_head_position Previous head position in the world, for the snake body update.
   */
  void UpdateBodyAndWorldView(const SDL_Point& prev_head_position);

  /**
   *  \brief Updates the snake's AI model based on the latest event (i.e. the result of its actions).
   */
  void Learn();

  /**
   *  \brief Calculates the snake's AI model decision for the next snake action, based on the world state.
   */
  void DefineAction();

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
   *  \brief Updates the snake front vision, due to the change of direction (i.e. left or right of previous direction).
   */
  void TurnEyes();

  /**
   *  \brief Returns the position of a given world point from the snake's perspective, relative to its head.
   *  \param point A position in the game screen, from the player's viewpoint.
   *  \return The equivalent point position from the snake's perspective.
   */
  SDL_Point ToSnakeVision(const SDL_Point& point) const;

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
  float speed{INITIAL_SNAKE_SPEED};

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
  bool automode{false};

  /**
   *  \brief Struct type representing a snake's perception of the world around it.
   */
  struct Vision {
    /**
     *  \brief Location of the snake's head used as base reference for its world view. From the snake's perspective, its head is on the middle-bottom of the vision grid, allowing it to see what lies ahead and by its sides.
     */
    const SDL_Point head;

    /**
     *  \brief Location of the tile right in front of the snake's head, from its perspective.
     */
    const SDL_Point front_tile;

    /**
     *  \brief Snake's world view in matricial form.
     */
    //std::vector<std::vector<WorldElement>> world;
    Matrix world;
  };

  /**
   *  \brief The snake's perception of the world around it.
   */
  struct Vision vision;

  /**
   *  \brief Random number generator defining the snake direction changes during auto mode.
   */
  std::default_random_engine generator;

  /**
   *  \brief Uniform real distribution to be used during calculation of snake direction changes during auto mode.
   */
  std::uniform_real_distribution<float> random_direction_distribution{0.0, 1.0};

  Scope root;
  std::unique_ptr<ClientSession> session;
  Output advance_input;
  Output advance_op;
  Output turn_right_input;
  Output turn_right_op;
  Output turn_left_input;
  Output turn_left_op;
};

#endif