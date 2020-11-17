#include "snake.h"
#include <cmath>
#include <iostream>

void Snake::Update() {
  SDL_Point prev_cell(head);  // We first capture the head's cell before updating.
  UpdateHead();

  // Update all of the body vector items if the snake head has moved to a new
  // cell.
  if (head.x != prev_cell.x || head.y != prev_cell.y) {
    UpdateBody(prev_cell);
  }
}

void Snake::UpdateHead() {
  switch (direction) {
    case Direction::kUp:
      head_y -= speed;
      break;

    case Direction::kDown:
      head_y += speed;
      break;

    case Direction::kLeft:
      head_x -= speed;
      break;

    case Direction::kRight:
      head_x += speed;
      break;
  }

  // Wrap the Snake around to the beginning if going off of the screen.
  head_x = fmod(head_x + grid_width, grid_width);
  head_y = fmod(head_y + grid_height, grid_height);

  head.x = static_cast<int>(head_x);
  head.y = static_cast<int>(head_y);
}

void Snake::UpdateBody(const SDL_Point &prev_head_cell) {
  // Add previous head location to vector
  body.push_back(prev_head_cell);

  if (!growing) {
    // Remove the tail from the vector.
    body.erase(body.begin());
  } else {
    growing = false;
    size++;
  }

  // Check if the snake has died.
  for (auto const &item : body) {
    if (head.x == item.x && head.y == item.y) {
      alive = false;
    }
  }
}

void Snake::GrowBody() { growing = true; }

// Inefficient method to check if cell is occupied by snake.
bool Snake::SnakeCell(int x, int y) {
  if (x == head.x && y == head.y) {
    return true;
  }
  for (auto const &item : body) {
    if (x == item.x && y == item.y) {
      return true;
    }
  }
  return false;
}

void Snake::ToggleAutoMode() {
  automode = !automode;
}

void Snake::Resurrect() {
  if (!alive) {
    speed = INITIAL_SNAKE_SPEED;
    size = 1;
    alive = true;
    body.clear();
    growing = false;
  }
}

SDL_Point Snake::ToSnakeVision(const SDL_Point& point) {
  SDL_Point distance_to_head;
  distance_to_head.x = point.x - head.x;
  distance_to_head.y = point.y - head.y;

  /* Convert to a position relative to the snake's head, from its perspective. */
  SDL_Point new_point;
  switch (direction) {
    int aux;
    case Direction::kDown:
      distance_to_head.x = -distance_to_head.x;
      distance_to_head.y = -distance_to_head.y;
      break;
    case Direction::kRight:
      aux = distance_to_head.x;
      distance_to_head.x = distance_to_head.y;
      distance_to_head.y = -aux;
      break;
    case Direction::kLeft:
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
  for (const SDL_Point& section : body) {
    vision.body.push_back(ToSnakeVision(section));
  }
}