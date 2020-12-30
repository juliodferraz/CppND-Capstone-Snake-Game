#include "snake.h"
#include <cmath>
#include <iostream>

void Snake::Update() {
  SDL_Point prev_cell(position.head);  // We first capture the head's cell before updating.
  UpdateHead();

  // Update all of the body vector items if the snake head has moved to a new
  // cell.
  if (position.head.x != prev_cell.x || position.head.y != prev_cell.y) {
    // Store snake's move event.
    event = Event::Moved;

    UpdateBody(prev_cell);

    /* In case the snake is autonomous, it might change direction randomly upon entering a new position. */
    if (automode) {
      float number = random_direction_distribution(generator);

      if (number < 0.33) {
        /* One third of chance to move left from current direction */
        Act(Action::MoveLeft);
      } else if (number < 0.66) {
        /* One third of chance to move right from current direction */
        Act(Action::MoveRight);
      } else {
        /* One third of chance to maintain current direction and move forward */
        Act(Action::MoveFwd);
      }
    }
  }
}

void Snake::UpdateHead() {
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
  head_x = fmod(head_x + grid_width, grid_width);
  head_y = fmod(head_y + grid_height, grid_height);

  position.head.x = static_cast<int>(head_x);
  position.head.y = static_cast<int>(head_y);
}

void Snake::UpdateBody(const SDL_Point &prev_head_cell) {
  // Add previous head location to vector
  position.body.push_back(prev_head_cell);

  if (!growing) {
    // Remove the tail from the vector.
    position.body.erase(position.body.begin());
  } else {
    growing = false;
    size++;
  }

  // Check if the snake has died.
  for (auto const &item : position.body) {
    if (position.head.x == item.x && position.head.y == item.y) {
      // Store collision event.
      event = Event::Collided;

      // Set snake as deceased.
      alive = false;
    }
  }
}

void Snake::HandleCommand(const Controller::UserCommand& command) {
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
  switch(action) {
    case Action::MoveLeft:
      direction = GetLeftOf(direction);
      break;
    case Action::MoveRight:
      direction = GetRightOf(direction);
      break;
    default:
      // Action::MoveFwd
      // Do nothing
      break;
  }
}

void Snake::Eat() { 
  // Store eating event.
  event = Event::Ate;

  // Set the snake's growth.
  growing = true;

  // Increase snake's speed.
  speed += 0.02;
}

// Inefficient method to check if cell is occupied by snake.
bool Snake::SnakeCell(const int& x, const int& y) const {
  if (x == position.head.x && y == position.head.y) {
    return true;
  }
  for (auto const &item : position.body) {
    if (x == item.x && y == item.y) {
      return true;
    }
  }
  return false;
}

void Snake::Resurrect() {
  if (!alive) {
    speed = INITIAL_SNAKE_SPEED;
    size = 1;
    alive = true;
    position.body.clear();
    growing = false;
    event = Event::Moved;
    action = Action::MoveFwd;
  }
}

SDL_Point Snake::ToSnakeVision(const SDL_Point& point) const {
  SDL_Point distance_to_head;
  distance_to_head.x = point.x - position.head.x;
  distance_to_head.y = point.y - position.head.y;

  /* Convert to a position relative to the snake's head, from its perspective. */
  SDL_Point new_point;
  switch (direction) {
    int aux;
    case Direction::Down:
      distance_to_head.x = -distance_to_head.x;
      distance_to_head.y = -distance_to_head.y;
      break;
    case Direction::Right:
      aux = distance_to_head.x;
      distance_to_head.x = distance_to_head.y;
      distance_to_head.y = -aux;
      break;
    case Direction::Left:
      aux = distance_to_head.x;
      distance_to_head.x = -distance_to_head.y;
      distance_to_head.y = aux;
      break;
    default: // Direction::kUp
      /* No mapping factor needed */
      break;
  }
  new_point.x = (distance_to_head.x + vision.head.x + grid_width) % grid_width;
  new_point.y = (distance_to_head.y + vision.head.y + grid_width) % grid_width;
  
  return new_point;
}

void Snake::SeeWorld(const SDL_Point& food) {
  /* Update vision of food. */
  vision.food = ToSnakeVision(food);

  /* Update vision of body. */
  vision.body.clear();
  for (const SDL_Point& section : position.body) {
    vision.body.push_back(ToSnakeVision(section));
  }
}

void Snake::Learn() {
  /* TODO: update the snake's AI model based on Snake::event */
}

void Snake::DefineAction() {
  /* TODO: run the snake's AI model based on its world view and update Snake::action */
}

Snake::Direction Snake::GetLeftOf(const Snake::Direction& reference) {
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 3) % 4); 
}

Snake::Direction Snake::GetRightOf(const Snake::Direction& reference) { 
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 1) % 4); 
}