#ifndef CONTROLLER_H
#define CONTROLLER_H

/**
 *  \brief Class responsible for processing the user inputs and returning any valid commands.
 */
class Controller {
 public:
  /**
   *  \brief User command enum, representing the possible player commands:
   * - Quit: user tried to close the game window;
   * - GoUp: up arrow key;
   * - GoRight: right arrow key;
   * - GoDown: down arrow key;
   * - GoLeft: left arrow key;
   * - ToggleAutoMode: "a" key (toggles auto snake control on and off);
   * - ToggleFpsCtrl: "s" key (toggles the frame rate control on and off);
   * - Pause/resume game: "p" key;
   * - Erase/reset game history and AI learning: "e" key;
   * - None: default value (i.e. no valid command has been issued).
   */
  enum class UserCommand { None, Quit, GoUp, GoRight, GoDown, GoLeft, 
    ToggleAutoMode, ToggleFpsCtrl, Pause, EraseData };

  /**
   *  \brief Process the user input and returns the latest command issued to the game.
   *  \return The latest player command.
   */
  UserCommand ReceiveCommand();  
};

#endif