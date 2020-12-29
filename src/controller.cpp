#include "controller.h"
#include <iostream>
#include "SDL.h"
#include "snake.h"

void Controller::ChangeDirection(Snake &snake, Snake::Direction input,
                                 Snake::Direction opposite) const {
  if (snake.direction != opposite) {
    /* Set current snake action (i.e. move forward, left or right) according to user input */
    if (input == snake.direction) {
      snake.SetAction(Snake::Action::MoveFwd);
    } else if (input == Snake::GetLeftOfDirection(snake.direction)) {
      snake.SetAction(Snake::Action::MoveLeft);
    } else {
      snake.SetAction(Snake::Action::MoveRight);
    }

    // Update the snake direction to the user input.
    snake.direction = input;
  }
  return;
}

void Controller::HandleInput(bool &running, Snake &snake) {
  /* Reset current snake action to default one. */
  snake.SetAction(Snake::Action::MoveFwd);

  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT) {
      running = false;
    } else if (e.type == SDL_KEYDOWN) {
      if (snake.IsAutoModeOn()) {
        /* In case the snake is autonomous, the only key available is 'a', used to toggle back to manual mode. */
        if (e.key.keysym.sym == SDLK_a) snake.ToggleAutoMode();
      } else {
        switch (e.key.keysym.sym) {
          case SDLK_UP:
            ChangeDirection(snake, Snake::Direction::kUp,
                            Snake::Direction::kDown);
            break;

          case SDLK_DOWN:
            ChangeDirection(snake, Snake::Direction::kDown,
                            Snake::Direction::kUp);
            break;

          case SDLK_LEFT:
            ChangeDirection(snake, Snake::Direction::kLeft,
                            Snake::Direction::kRight);
            break;

          case SDLK_RIGHT:
            ChangeDirection(snake, Snake::Direction::kRight,
                            Snake::Direction::kLeft);
            break;

          case SDLK_a:
            snake.ToggleAutoMode();
            break;
        }
      }
    }
  }

  /* In case the snake is autonomous, it might change direction randomly and automatically. */
  if (snake.IsAutoModeOn()) {
    float number = random_direction_distribution(generator);

    if (number < 0.33) {
      /* One third of chance to move left from current direction */
      snake.direction = Snake::GetLeftOfDirection(snake.direction);
      snake.SetAction(Snake::Action::MoveLeft);

    } else if (number < 0.66) {
      /* One third of chance to move right from current direction */
      snake.direction = Snake::GetRightOfDirection(snake.direction);
      snake.SetAction(Snake::Action::MoveRight);

    } else {
      /* One third of chance to maintain current direction and move forward */
      /* No operation needed */
      snake.SetAction(Snake::Action::MoveFwd);
    }
  }
}