#ifndef WORLD_H
#define WORLD_H

#include <random>
#include <unordered_map>
#include <deque>

#include "controller.h"
#include "coords2D.h"

#include "SDL.h"

/**
 *  \brief Class managing the world grid/scenario of the game and all non-controllable elements (e.g. walls and food).
 */
class World {
 public:
  /**
   *  \brief Enum type representing the possible contents of a tile in the world grid.
   */
  enum class Element { None, SnakeHead, SnakeBody, Wall, Food};

  /**
   *  \brief Constructor of the World class. The world is initialized with its walls and a food.
   *  \param gridSideLen The square world's width/height, in number of grid cells.
   */
  World(const unsigned int gridSideLen);

  /**
   *  \brief Clean and re-initializes the world grid and the food in it.
   */
  void Init();

  /**
   *  \brief Places a new food in the world, in an available empty location.
   *  \return True, if a food was able to be placed in the world; false, if no empty grid cell was available.
   */
  bool GrowFood();

  /**
   *  \brief Returns a const reference to the current food position in the world.
   *  \return Reference to current food position.
   */
  inline const SDL_Point& GetFoodPosition() const { return food; }

  /**
   *  \brief Returns a boolean indicating if there's an obstacle (e.g. wall, snake part or out-of-world-grid-boundaries) 
   * in the input position.
   *  \return True, if input position holds an obstacle, with which a collision leads to the snake's death; false, otherwise.
   */
  bool IsObstacle(const SDL_Point& position) const;

  /**
   *  \brief Updates the element located in a specific world grid tile. 
   * If the position is outside grid boundaries, a runtime exception is raised.
   *  \param position The target position.
   *  \param element The new element to be set at this position.
   */
  void SetElement(const SDL_Point& position, const World::Element element);

  /**
   *  \brief Returns the current content of a specific tile in the world grid.
   * If the position is outside grid boundaries, a runtime exception is raised.
   *  \param position The target position.
   *  \return Passed-by-value Element located in the input position.
   */
  World::Element GetElement(const SDL_Point& position) const;

  /**
   *  \brief Returns the current content of a specific tile in the world grid.
   * If the position is outside grid boundaries, a runtime exception is raised.
   *  \param position The target position.
   *  \return Passed-by-reference Element located in the input position.
   */
  World::Element& GetElementRef(const SDL_Point& position);

 private:
  /**
   *  \brief Cleans and re-initializes the world grid.
   */
  void InitWorldGrid();

  /**
   *  \brief Checks if a point is located inside the world grid boundaries.
   *  \param position The point position.
   *  \return True, if the point is located inside grid boundaries; false, otherwise.
   */
  bool IsInsideBoundaries(const SDL_Point& position) const;

  /**
   *  \brief The world grid, indicating the world elements in matricial format.
   */
  std::vector<std::vector<Element>> grid;

  /**
   *  \brief A container indicating all currently empty grid positions, indexed by the Element object address.
   * The key value is the grid Element address, while the mapped value is the position in the grid, necessary
   * to know where to initialize the food, when this map is used to assess the empty grid positions.
   */
  std::unordered_map<Element*,SDL_Point> freeGridPositions;

  /**
   *  \brief The length of the world grid side in number of cells.
   */
  const unsigned int gridSideLen;

  /**
   *  \brief The location of the food in the world grid.
   */
  SDL_Point food;

  /**
   *  \brief Random number generator. Initialized in class constructor with the system clock as a seed.
   */
  std::default_random_engine randGenerator;
};

#endif