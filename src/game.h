#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "world.h"

/**
 *  \brief Class responsible for managing the game's mechanics and rules.
 */
class Game {
 public:
  Game(const std::size_t& grid_side_size);
  void Run(Controller &controller, Renderer &renderer,
           const std::size_t& target_frame_duration);
  int GetScore() const { return world.GetSnake().GetSize() - 1; }

 private:
  World world;

  /**
   *  \brief Updates the game state according to the user command.
   *  \param command Latest command issued by the player.
   */
  void ProcessCommand(const Controller::UserCommand& command);

  /**
   *  \brief Flag indicating if game is running.
   */
  bool running{false};
};

#endif