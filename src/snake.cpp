#include "snake.h"
#include <cmath>
#include <iostream>
#include <algorithm>

Snake::Snake(const SDL_Point& startPosition, World& world) 
  : head{startPosition}, world{world}, positionQueue{{startPosition}} {
  // Initialize snake head tile in world.
  this->world.SetElement(head, World::Element::SnakeHead);
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
        if (forbiddenDir != Direction::Up) {
          if (direction == Direction::Left) Act(Action::MoveRight);
          else if (direction == Direction::Right) Act(Action::MoveLeft);
          else Act(Action::MoveFwd);
        }
        break;
      case Controller::UserCommand::GoDown:
        if (forbiddenDir != Direction::Down) {
          if (direction == Direction::Left) Act(Action::MoveLeft);
          else if (direction == Direction::Right) Act(Action::MoveRight);
          else Act(Action::MoveFwd);
        }
        break;
      case Controller::UserCommand::GoLeft:
        if (forbiddenDir != Direction::Left) {
          if (direction == Direction::Up) Act(Action::MoveLeft);
          else if (direction == Direction::Down) Act(Action::MoveRight);
          else Act(Action::MoveFwd);
        }
        break;
      case Controller::UserCommand::GoRight:
        if (forbiddenDir != Direction::Right) {
          if (direction == Direction::Up) Act(Action::MoveRight);
          else if (direction == Direction::Down) Act(Action::MoveLeft);
          else Act(Action::MoveFwd);
        }
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
void Snake::Init(const SDL_Point& startPosition) {
  // Reset all snake parameters.
  alive = true;
  event = Event::SameTile;
  action = Action::MoveFwd;
  direction = Direction::Up;
  forbiddenDir = Direction::Down;

  head = Coords2D(startPosition);
  positionQueue.clear();
  positionQueue.push_front(head);
  // Initialize snake head tile in world.
  world.SetElement(head, World::Element::SnakeHead);
  
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

  // Update the snake position queue if needed, depending on the event.
  switch (this->event) {
    case Event::Collided:
      alive = false;
      break;
    case Event::NewTile:
      // Remove the previous tail position from the world grid, as the snake didn't grow.
      PopSnakeTailPos();
      // Set the current head position as the target one.
      PushNewSnakeHeadPos(head);
      // Set opposite to current direction as forbidden so that the snake cannot move to the same tile of its
      // first body part.
      UpdateForbiddenDir();
      break;
    case Event::Ate:
      // Set the current head position as the target one.
      PushNewSnakeHeadPos(head);
      // Set opposite to current direction as forbidden so that the snake cannot move to the same tile of its
      // first body part.
      UpdateForbiddenDir();
      break;
    default:
      // Event::SameTile
      // No need to update snake position queue.
      break;
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

void Snake::PopSnakeTailPos() {
  world.SetElement(GetTailPosition(), World::Element::None);
  positionQueue.pop_back(); 
}

void Snake::PushNewSnakeHeadPos(const SDL_Point& head) {
  if (GetSize() > 0) {
    // If the snake has a body (or is about to have, with the new head), 
    // update the previous head position to contain a snake body part.
    world.SetElement(GetHeadPosition(), World::Element::SnakeBody);
  }
  positionQueue.push_front(head); 
  world.SetElement(head, World::Element::SnakeHead);
}