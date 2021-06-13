#include "game.h"
#include <iostream>
#include "SDL.h"

Game::Game(const std::size_t& grid_side_size) : world(grid_side_size) {
  #if DEBUG_MODE
    std::cout << "Game object created" << std::endl;
  #endif
}

void Game::Run(Controller &controller, Renderer &renderer, const std::size_t& target_frame_duration) {
  Uint32 title_timestamp = SDL_GetTicks();
  Uint32 frame_start;
  Uint32 frame_end;
  Uint32 frame_duration;
  int frame_count = 0;
  running = true;

  while (running) {
    frame_start = SDL_GetTicks();

    // Input, Update, Render - the main game loop.
    ProcessCommand(controller.ReceiveCommand());
    world.Update();
    renderer.Render(world);

    frame_end = SDL_GetTicks();

    // Keep track of how long each loop through the input/update/render cycle
    // takes.
    frame_count++;
    frame_duration = frame_end - frame_start;

    // After every second, update the window title.
    if (frame_end - title_timestamp >= 1000) {
      renderer.UpdateWindowTitle(this->GetScore(), frame_count, world.GetSnake().IsAutoModeOn());
      frame_count = 0;
      title_timestamp = frame_end;
    }

    // If the time for this frame is too small (i.e. frame_duration is
    // smaller than the target ms_per_frame), delay the loop to
    // achieve the correct frame rate.
    if (frame_duration < target_frame_duration) {
      SDL_Delay(target_frame_duration - frame_duration);
    }

    /* If, after the current game frame, the snake is deceased, wait 1 second and then reset the game (including the score). */
    if (!world.GetSnake().IsAlive()) {
      //SDL_Delay(1000);
      //world.Reset();
    }
  }
}

void Game::ProcessCommand(const Controller::UserCommand& command) {
  if (command == Controller::UserCommand::Quit) {
    running = false;
  } else {
    // For any other command (other than game quit), pass the command on to be processed by the snake object.
    world.CommandSnake(command);
  }
}