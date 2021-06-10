#include "snake.h"
#include <cmath>
#include <iostream>
#include <algorithm>

Snake::Snake(const int& grid_side_size, const unsigned int& layer1_size, const unsigned int& layer2_size)
  : grid_side_size(grid_side_size),
    head_x(grid_side_size / 2),
    head_y(grid_side_size / 2),
    position{{static_cast<int>(head_x), static_cast<int>(head_y)}, std::vector<SDL_Point>{}} {
  #if DEBUG_MODE
    std::cout << "Snake object created" << std::endl;
  #endif
}

void Snake::Move() {
  switch (direction) {
    case Direction::Up:
      head_y -= speed;
      break;

    case Direction::Down:
      head_y += speed;
      break;

    case Direction::Left:
      head_x -= speed;
      break;

    case Direction::Right:
      head_x += speed;
      break;
  }

  // Wrap the Snake around to the beginning if going off of the screen.
  // TODO: repeated operation
  head_x = fmod(head_x + grid_side_size, grid_side_size);
  head_y = fmod(head_y + grid_side_size, grid_side_size);

  position.head.x = static_cast<int>(head_x);
  position.head.y = static_cast<int>(head_y);

  #if DEBUG_MODE
    std::cout << "Snake head moved!" << std::endl;
  #endif
}

void Snake::ProcessUserCommand(const Controller::UserCommand& command) {
  if(command == Controller::UserCommand::ToggleAutoMode) ToggleAutoMode();
  else if(!automode) {
    // If auto mode is on, only the auto mode toggling command is available, and all other are ignored.
    switch(command) {
      case Controller::UserCommand::GoUp:
        if (direction == Direction::Left) Act(Action::MoveRight);
        else if (direction == Direction::Right) Act(Action::MoveLeft);
        else Act(Action::MoveFwd);
        break;
      case Controller::UserCommand::GoDown:
        if (direction == Direction::Left) Act(Action::MoveLeft);
        else if (direction == Direction::Right) Act(Action::MoveRight);
        else Act(Action::MoveFwd);
        break;
      case Controller::UserCommand::GoLeft:
        if (direction == Direction::Up) Act(Action::MoveLeft);
        else if (direction == Direction::Down) Act(Action::MoveRight);
        else Act(Action::MoveFwd);
        break;
      case Controller::UserCommand::GoRight:
        if (direction == Direction::Up) Act(Action::MoveRight);
        else if (direction == Direction::Down) Act(Action::MoveLeft);
        else Act(Action::MoveFwd);
        break;
      default:
        // UserCommand::None (no command issued by user)
        break;
    }
  }
}

void Snake::Act(const Action& input) {
  action = input;
  if(action == Action::MoveFwd) {
    // Do nothing
  } else {
    if(action == Action::MoveLeft) {
      direction = GetLeftOf(direction);
    } else {
      // Action::MoveRight
      direction = GetRightOf(direction);
    }
  }
}

// TODO: desvincular Init method da inicialização da comida na visão da cobra, e incluir Init no construtor tambem
void Snake::Init(const SDL_Point& food_position) {
  // Reset all snake parameters.
  alive = true;
  position.body.clear();
  size = 1;
  speed = INITIAL_SNAKE_SPEED;
  event = Event::SameTile;
  action = Action::MoveFwd;

  #if DEBUG_MODE
    std::cout << "Snake initiated!" << std::endl;
  #endif
}

void Snake::DefineAction() {
  /* TODO: implement deterministic algortithm for snake's next action definition. */

  if (automode) {
    // If auto mode is active, sets the snake's next action at random.
    // Random decision model
    float number = random_direction_distribution(generator);

    if (number < 0.33) {
      /* Chance to move left from current direction */
      Act(Action::MoveLeft);
      std::cout << "Move Left!" << std::endl;
    } else if (number < 0.66) {
      /* Chance to move right from current direction */
      Act(Action::MoveRight);
      std::cout << "Move Right!" << std::endl;
    } else {
      /* Chance to maintain current direction and move forward */
      Act(Action::MoveFwd);
      std::cout << "Move Forward!" << std::endl;
    }
  }
}

Snake::Direction Snake::GetLeftOf(const Snake::Direction& reference) {
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 3) % 4); 
}

Snake::Direction Snake::GetRightOf(const Snake::Direction& reference) { 
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 1) % 4); 
}

void Snake::SetEvent(const Event& event) {
  this->event = event;

  if (this->event == Event::Collided) {
    alive = false;
  } else if (this->event == Event::Ate) {
    // Increase snake's size and speed.
    size++;
    speed += 0.02;
  }
}

void Snake::UpdateBody(const SDL_Point& prev_head_position) {
  // Update the snake body location, if it has one.
  if (size > 1) {
    // If the snake has a body, add previous head location to the body vector.
    position.body.push_back(prev_head_position);

    // Next, in case the snake didn't eat, move the oldest body vector item (i.e. the snake's tail).
    if (event != Snake::Event::Ate) {
      // Remove the tail from the body vector.
      position.body.erase(position.body.begin());
    }
  }
}