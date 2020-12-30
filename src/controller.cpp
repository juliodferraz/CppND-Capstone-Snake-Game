#include "controller.h"
#include <iostream>
#include "SDL.h"

Controller::UserCommand Controller::ReceiveCommand() {
  UserCommand command{UserCommand::None};

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      command = UserCommand::Quit;
    } else if (e.type == SDL_KEYDOWN) {
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
      }
    }
  }

  return command;
}