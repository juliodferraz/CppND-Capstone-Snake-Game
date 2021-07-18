#ifndef WORLD_H
#define WORLD_H

#include <random>
#include <unordered_map>
#include <memory>
#include <utility>
#include <deque>

#include "controller.h"
#include "build.h"
#include "matrix.h" // TODO: confirm this is being used, otherwise remove it.
#include "coords2D.h"

#include "SDL.h"

/**
 *  \brief Class representing the world of the game, owning its scenario and inhabitants (i.e. the snake and fruits).
 */
class World {
 public:
  /**
   *  \brief Enum type representing the possible contents of a tile in the world grid.
   */
  enum class Element { None = 0, SnakeHead = -1, SnakeBody = -2, Wall = -3, Food = 2};

  /**
   *  \brief Constructor of the World class. The world starts empty, with no snake nor fruit.
   *  \param grid_side_size The square world's width/height, in terms of grid cells.
   */
  World(const std::size_t& grid_side_size);

  /**
   *  \brief Gives birth to a new snake in the world, replacing the previous one, if any.
   */
  void Reset();

  /**
   *  \brief Returns a const reference to the current food position in the world.
   *  \return Reference to current food position.
   */
  const SDL_Point& GetFoodPosition() const { return food; }

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
  void InsertSnake(const SDL_Point& head);
  void PopSnakeTail();
  void PushSnakeHead(const SDL_Point& head);

  // TODO: comment
  bool IsObstacle(const SDL_Point& position) const;

  /**
   *  \brief Places a new fruit in the world, in an available empty location.
   */
  void GrowFood();

  /**
   *  \brief Returns the current content of a specific tile in the world grid.
   *  \return Element located in the input position.
   */
  inline World::Element GetElement(const SDL_Point& position) const { return grid[position.y][position.x]; }

 private:
  /**
   *  \brief Updates the element located in a specific world grid tile.
   *  \param position The target position.
   *  \param new_element The new element to be set.
   */
  void SetElement(const SDL_Point& position, const World::Element& new_element);

  /**
   *  \brief Initializes the world grid and its elements.
   */
  void InitWorldGrid();

  // TODO: comment
  int DistanceToFood(const SDL_Point& position) const;

  /**
   *  \brief The world grid, indicating the world elements in matricial format.
   */
  std::vector<std::vector<Element>> grid;
  std::unordered_map<Element*,SDL_Point> freeGridPositions;
  int grid_side_size;

  /**
   *  \brief Double-ended queue containing the snake's head and body parts coordinates in the world. The double-ended queue
   * has constant complexity for push and pop operations at both queue ends, which makes it more efficient to be used here
   * instead of a vector (which displays linear complexity for operations at its front).
   */
  std::deque<SDL_Point> positionQueue;

  SDL_Point food;

  std::random_device dev;
  std::mt19937 engine;

  std::default_random_engine generator;
  std::uniform_real_distribution<float> random_direction_distribution{0.0, 1.0};
};

#endif