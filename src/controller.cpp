#include "controller.h"
#include "SDL.h"

Controller::UserCommand Controller::ReceiveCommand() {
  UserCommand command{UserCommand::None};

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      // If latest command consisted of closing the game window, return the Quit command.
      command = UserCommand::Quit;
    } else if (e.type == SDL_KEYDOWN) {
      // Otherwise, if a key has been pressed, identify which key it was and return the appropriate command.
      switch (e.key.keysym.sym) {
        case SDLK_UP:
          command = UserCommand::GoUp;
          break;

        case SDLK_DOWN:
          command = UserCommand::GoDown;
          break;

        case SDLK_LEFT:
          command = UserCommand::GoLeft;
          break;

        case SDLK_RIGHT:
          command = UserCommand::GoRight;
          break;

        case SDLK_a:
          command = UserCommand::ToggleAutoMode;
          break;

        case SDLK_s:
          command = UserCommand::ToggleFpsCtrl;
          break;

        default:
          // No valid command
          command = UserCommand::None;
          break;
      }
    }
  }

  return command;
}