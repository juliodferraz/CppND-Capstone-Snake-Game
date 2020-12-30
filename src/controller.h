#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "snake.h"

/**
 *  \brief Class responsible for processing the user inputs and indicating the issued commands.
 */
class Controller {
 public:
  /**
   *  \brief User command enum, representing the possible player commands.
   */
  enum class UserCommand { None, Quit, GoUp, GoRight, GoDown, GoLeft, ToggleAutoMode };

  /**
   *  \brief Process the user input and returns the latest command issued to the game.
   *  \return The latest player command.
   */
  UserCommand ReceiveCommand();

 private:
  
};

#endif