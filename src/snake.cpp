#include "snake.h"
#include <cmath>
#include <iostream>
#include <algorithm>

Snake::Snake(const int& grid_side_size, const unsigned int& layer1_size, const unsigned int& layer2_size)
  : grid_side_size(grid_side_size),
    head{(float) grid_side_size/2, (float) grid_side_size/2},
    body{std::deque<SDL_Point>({head})} {
  #if DEBUG_MODE
    std::cout << "Snake object created" << std::endl;
  #endif
}

void Snake::Move() {
  switch (direction) {
    case Direction::Up:
      head += SDL_FPoint{0,-speed};
      break;

    case Direction::Down:
      head += SDL_FPoint{0,speed};
      break;

    case Direction::Left:
      head += SDL_FPoint{-speed,0};
      break;

    case Direction::Right:
      head += SDL_FPoint{speed,0};
      break;
  }

  // Wrap the Snake around to the beginning if going off of the screen.
  // TODO: repeated operation
  head = SDL_FPoint{(float) fmod(head.GetRealX() + grid_side_size, grid_side_size),
                    (float) fmod(head.GetRealY() + grid_side_size, grid_side_size)};

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
void Snake::Init() {
  // Reset all snake parameters.
  alive = true;
  size = 1;
  event = Event::SameTile;
  action = Action::MoveFwd;

  head = Coords2D((float) grid_side_size/2, (float) grid_side_size/2);
  body.clear();
  body.push_front(head);
  
  #if DEBUG_MODE
    std::cout << "Snake initiated!" << std::endl;
  #endif
}

Snake::Direction Snake::GetLeftOf(const Snake::Direction& reference) {
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 3) % 4); 
}

Snake::Direction Snake::GetRightOf(const Snake::Direction& reference) { 
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 1) % 4); 
}

Snake::Direction Snake::GetOppositeOf(const Snake::Direction& reference) { 
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 2) % 4); 
}

void Snake::SetEvent(const Event& event) {
  this->event = event;

  if (this->event == Event::Collided) {
    alive = false;
  } else if (this->event == Event::Ate) {
    // Increase snake's size.
    size++;
  }
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

bool Snake::SetDirection(const Direction& direction) {
  if (direction != GetOppositeOf(this->direction)) {
    // Protects against changing to the direction contrary to the current one.
    // As the snake can only move forward, turn right or turn left.
    this->direction = direction;
    return true;
  }
  else return false;
}

void Snake::UpdateBody() {
  // Push the new head location to the front of the body queue.
  body.push_front(head);

  // Next, in case the snake didn't eat, move the oldest body vector item (i.e. the snake's tail).
  if (event != Snake::Event::Ate) {
    // Remove the tail from the body vector.
    body.pop_back();
  }
}