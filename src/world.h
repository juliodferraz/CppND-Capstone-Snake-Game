#ifndef WORLD_H
#define WORLD_H

#include <random>
#include "snake.h"
#include "SDL.h"
#include "controller.h"

/**
 *  \brief Class representing the world of the game, owning its scenario and inhabitants (i.e. the snake and fruits).
 */
class World {
 public:
  /**
   *  \brief Constructor of the World class. The world starts empty, with no snake nor fruit.
   *  \param grid_width The world's width, in terms of grid cells.
   *  \param grid_height The world's height, in terms of grid cells.
   */
  World(const std::size_t& grid_width, const std::size_t& grid_height);

  // TODO: Matrix GetWorldState() const { return state; }

  /**
   *  \brief Updates the world and its inhabitants states.
   */
  void Update();

  /**
   *  \brief Gives birth to a new snake in the world, replacing the previous one, if any.
   */
  void CreateSnake();

  /**
   *  \brief Issue user command to the snake.
   *  \param command Latest command issued by the player.
   */
  void CommandSnake(const Controller::UserCommand& command) { snake.HandleCommand(command); }

  /**
   *  \brief Makes the snake grow (one grid cell of growth).
   */
  void GrowSnake();

  /**
   *  \brief Kills the current snake, if any, setting its internal state as "deceased".
   */
  void KillSnake();

  /**
   *  \brief Places a new fruit in the world, in an available empty location.
   */
  void GrowFood();

  /**
   *  \brief Moves the snake and updates its location in the world.
   *  \return The resulting snake event, after its movement (i.e. collision, fruit-eating, or none).
   */
  Snake::Event MoveSnake();

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

 private:
  


  // TODO: declare "state", a matrix of signed integers
  // A -2 value in a cell, indicates a snake body part; -1, the snake's head; 0, an empty cell; and 2, a fruit.

  Snake snake;
  SDL_Point food;

  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<int> random_w;
  std::uniform_int_distribution<int> random_h;
};

#endif