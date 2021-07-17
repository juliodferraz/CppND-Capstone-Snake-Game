#ifndef GAME_H
#define GAME_H

#include "controller.h"
#include "renderer.h"
#include "world.h"
#include "snake.h"
#include "build.h"

#include "SDL.h"

/**
 *  \brief Class responsible for managing the game's mechanics and rules.
 */
class Game {
 public:
  Game(const std::size_t& grid_side_size);
  void Run(Controller &controller, Renderer &renderer,
           const std::size_t& target_frame_duration);
  int GetScore() const { return snake.GetSize() - 1; }

  /**
   *  \brief Returns a const reference to the snake object.
   *  \return Const reference to snake object.
   */
  const Snake& GetSnake() const { return snake; } // TODO: confirm this is being used somewhere, otherwise delete it.

 private:
  World world;
  Snake snake;

  /**
   *  \brief Updates the game state according to the user command.
   *  \param command Latest command issued by the player.
   */
  void ProcessCommand(const Controller::UserCommand& command);

  void Update();
  void Reset();

  /**
   *  \brief Flag indicating if game is running.
   */
  bool running{false};

  std::size_t grid_side_size;
};

#endif