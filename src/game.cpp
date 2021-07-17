#include "game.h"
#include <iostream>
#include "SDL.h"

Game::Game(const std::size_t& grid_side_size) 
  : grid_side_size{grid_side_size},
    world(grid_side_size),
    snake(SDL_Point{(int) grid_side_size/2, (int) grid_side_size/2}, world) {
  world.InsertSnake(snake.GetTargetHeadPosition());
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
    this->Update();
    renderer.Render(world, snake);

    frame_end = SDL_GetTicks();

    // Keep track of how long each loop through the input/update/render cycle
    // takes.
    frame_count++;
    frame_duration = frame_end - frame_start;

    // After every second, update the window title.
    if (frame_end - title_timestamp >= 1000) {
      renderer.UpdateWindowTitle(this->GetScore(), frame_count, snake.IsAutoModeOn());
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
    if (!snake.IsAlive()) {
      SDL_Delay(1000);
      this->Reset();
    }
  }
}

void Game::ProcessCommand(const Controller::UserCommand& command) {
  if (command == Controller::UserCommand::Quit) {
    running = false;
  } else {
    // For any other command (other than game quit), pass the command on to be processed by the snake object.
    snake.ProcessUserCommand(command);
  }
}

void Game::Update() {
  #if DEBUG_MODE
    std::cout << "World update begin" << std::endl;
  #endif

  // If the snake is deceased, no world update needs to be done.
  if (!snake.IsAlive()) return;

  // Otherwise, move the snake in its current direction.
  snake.Move();

  // Check if snake head is about to move to a new tile.
  SDL_Point targetHeadPosition{snake.GetTargetHeadPosition()};
  SDL_Point headPosition{world.GetHeadPosition()};
  SDL_Point tailPosition{world.GetTailPosition()};

  if (!(targetHeadPosition == headPosition)) {
    // Checks the new tile content and raises appropriate event (e.g. eating, collision, etc.)
    if (world.IsObstacle(targetHeadPosition)) {
      snake.SetEvent(Snake::Event::Collided);

    } else {
      if (world.GetElement(targetHeadPosition) == World::Element::Food) {
        snake.SetEvent(Snake::Event::Ate);
      } else {
        snake.SetEvent(Snake::Event::NewTile);
      }

      // If the snake has a body, update the current head position to contain a snake body part
      if (snake.GetSize() > 1) world.SetElement(headPosition, World::Element::SnakeBody);

      // Move the snake head in the world grid to its new position
      world.SetElement(targetHeadPosition, World::Element::SnakeHead);

      if (snake.GetEvent() == Snake::Event::Ate) {
        // Now that the food has been eaten, make new food appear in a free grid tile.
        world.GrowFood();
      } else {
        // Remove the previous tail position from the world grid, as the snake didn't grow.
        world.SetElement(tailPosition, World::Element::None);
      }

      // Set the snake tail element in the world grid.
      if (snake.GetSize() > 1) world.SetElement(world.GetTailPosition(), World::Element::SnakeTail);

      if (snake.IsAutoModeOn()) {
        // TODO: call the snake's decision model to define next direction.
      }
    }

  } else {
    // Snake head is still in the same world grid tile.
    snake.SetEvent(Snake::Event::SameTile);
  }

  #if DEBUG_MODE
    std::cout << "World updated" << std::endl;
  #endif
}

void Game::Reset() {
  // Initialize snake and its world perception.
  snake.Init(SDL_Point{(int) grid_side_size/2, (int) grid_side_size/2});

  // Re-initialize world.
  world.Reset();
}