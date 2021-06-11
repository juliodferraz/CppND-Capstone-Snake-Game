#ifndef WORLD_H
#define WORLD_H

#include <random>

#include "snake.h"
#include "controller.h"
#include "build.h"
#include "matrix.h"

#include "SDL.h"

/**
 *  \brief Class representing the world of the game, owning its scenario and inhabitants (i.e. the snake and fruits).
 */
class World {
 public:
  /**
   *  \brief Enum type representing the possible contents of a tile in the world grid.
   */
  enum class Element { None = 0, SnakeHead = -1, SnakeBody = -2, SnakeTail = -3, Wall = -4, Food = 2};

  /**
   *  \brief Constructor of the World class. The world starts empty, with no snake nor fruit.
   *  \param grid_side_size The square world's width/height, in terms of grid cells.
   */
  World(const std::size_t& grid_side_size);

  /**
   *  \brief Gives birth to a new snake in the world, replacing the previous one, if any.
   */
  void InitSnake();

  /**
   *  \brief Updates the world and its inhabitants states.
   */
  void Update();

  /**
   *  \brief Issue user command to the snake.
   *  \param command Latest command issued by the player.
   */
  void CommandSnake(const Controller::UserCommand& command) { snake.ProcessUserCommand(command); }

  /**
   *  \brief Returns a const reference to the snake object.
   *  \return Const reference to snake object.
   */
  const Snake& GetSnake() const { return snake; }

  /**
   *  \brief Returns a const reference to the current food position in the world.
   *  \return Reference to current food position.
   */
  const SDL_Point& GetFoodPosition() const { return food; }

  /**
   *  \brief Returns the current content of a specific tile in the world grid.
   *  \return Element located in the input position.
   */
  inline Element GetElement(const SDL_Point& position) const;
  
  /**
   *  \brief Updates the element located in a specific world grid tile.
   *  \param position The target position.
   *  \param new_element The new element to be set.
   */
  inline void SetElement(const SDL_Point& position, const Element& new_element);

 private:
  /**
   *  \brief Places a new fruit in the world, in an available empty location.
   */
  void GrowFood();

  /**
   *  \brief Initializes the world grid and its elements.
   */
  void InitWorldGrid();

  // TODO: comment
  int DistanceToFood(const SDL_Point& head_position);

  /**
   *  \brief The world grid, indicating the world elements in matricial format.
   */
  Matrix grid;

  int grid_side_size;

  Snake snake;
  SDL_Point food;

  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<int> random_w;
  std::uniform_int_distribution<int> random_h;
};

#endif