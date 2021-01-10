#include "snake.h"
#include <cmath>
#include <iostream>
#include <algorithm>

Snake::Snake(const int& grid_side_size)
  : grid_side_size(grid_side_size),
    head_x(grid_side_size / 2),
    head_y(grid_side_size / 2),
    position{{static_cast<int>(head_x), static_cast<int>(head_y)}, std::vector<SDL_Point>{}},
    vision{{(grid_side_size - 1) / 2, grid_side_size - 1}, 
      {(grid_side_size - 1) / 2, grid_side_size - 2}, 
      {grid_side_size, grid_side_size}},
    root{Scope::NewRootScope()},
    session(root),
    advance_operator(grid_side_size, grid_side_size),
    turn_operator_a(grid_side_size, grid_side_size),
    turn_operator_b(grid_side_size, grid_side_size),
    advance(root, advance_operator.GetTensor(), vision.world.GetTensor()),
    turn_right(root, turn_operator_a.GetTensor(), MatMul(root, vision.world.GetTensor(), turn_operator_b.GetTensor(), {true, false})),
    turn_left(root, turn_operator_b.GetTensor(), MatMul(root, vision.world.GetTensor(), turn_operator_a.GetTensor(), {true, false})) {
  // Initialize advance forward matricial operator
  int one_col = grid_side_size - 1; // Index of the column that shall have the value of 1 (one).
  for(int row = 0; row < grid_side_size; row++) {
    advance_operator(row, one_col) = 1;
    one_col = (one_col + 1) % grid_side_size;
  }

  // Initialize turn direction matricial operator A
  one_col = vision.head.x; // Index of the column that shall have the value of 1 (one).
  for(int row = vision.head.y; row >= 0; row--) {
    turn_operator_a(row, one_col) = 1;
    one_col = (one_col + 1) % grid_side_size;
  }

  // Initialize turn direction matricial operator B
  one_col = vision.head.x; // Index of the column that shall have the value of 1 (one).
  for(int row = vision.head.y; row >= 0; row--) {
    turn_operator_b(row, one_col) = 1;
    one_col = (one_col - 1 + grid_side_size) % grid_side_size;
  }

#if DEBUG_MODE
  std::cout << "Snake object created" << std::endl;
#endif
}

void Snake::Move() {
#if DEBUG_MODE
  std::cout << "Snake began to move..." << std::endl;
#endif

  SDL_Point prev_cell(position.head);  // We first capture the head's cell before updating.
  MoveHead();

  if (position.head.x != prev_cell.x || position.head.y != prev_cell.y) {
    // Snake head has moved to a new world grid tile.
    event = Event::NewTile;

    // Senses the front tile content and raises any event (e.g. eating, collision, etc.)
    SenseFrontTile();

    // Make snake AI model learn, based on the result of its latest action.
    // This learning process shall occur even when the player is controlling the snake, so that the
    // AI may also learn from observing the player himself.
    Learn();

    // Advance world view in one tile forward and update body
    UpdateBodyAndWorldView(prev_cell);

    // If auto mode is active, AI model decides the snake's next action.
    if(automode) DefineAction();

  } else {
    // Snake head is still in the same world grid tile.
    event = Event::SameTile;
  }

#if DEBUG_MODE
  std::cout << "Snake moved!" << std::endl;
#endif
}

void Snake::MoveHead() {
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

void Snake::SetWorldViewElement(const SDL_Point& position, const Snake::WorldElement& new_element) {
  // Convert ṕosition from player's to snake's perspective
  SDL_Point snake_view_position = ToSnakeVision(position);

  // Update in snake view grid
  vision.world(snake_view_position.y, snake_view_position.x) = static_cast<int>(new_element);
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

    // Update the snake front vision to the new perspective (due to the new direction).
    TurnEyes();
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
    default: // Direction::Up
      /* No mapping factor needed */
      break;
  }
  // TODO: repeated operation
  new_point.x = (distance_to_head.x + vision.head.x + grid_side_size) % grid_side_size;
  new_point.y = (distance_to_head.y + vision.head.y + grid_side_size) % grid_side_size;
  
  return new_point;
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
  vision.world.Reset();

  // Initialize the snake's world view based on the food and its body positions.
  // Initialize food tile.
  SetWorldViewElement(food_position, Snake::WorldElement::Food);

  // Initialize snake head tile.
  SetWorldViewElement(position.head, Snake::WorldElement::Head);

  // Initialize snake body tiles.
  for(const SDL_Point& body_part : position.body) {
    SetWorldViewElement(body_part, Snake::WorldElement::Body);
  }

#if DEBUG_MODE
  std::cout << "Snake initiated!" << std::endl;
#endif
}

void Snake::Learn() {
  /* TODO: update the snake's AI model based on Snake::event */
}

void Snake::DefineAction() {
  /* TODO: run the snake's AI model based on its world view and update Snake::action */
  
  // Random decision model
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

Snake::Direction Snake::GetLeftOf(const Snake::Direction& reference) {
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 3) % 4); 
}

Snake::Direction Snake::GetRightOf(const Snake::Direction& reference) { 
  return static_cast<Snake::Direction>((static_cast<uint8_t>(reference) + 1) % 4); 
}

void Snake::SenseFrontTile() {
#if DEBUG_MODE
  std::cout << "Snake began sensing front tile..." << std::endl;
#endif

  // Check if the tile where the snake head moved to contain a body part or food, and raise event.
  if (GetWorldViewElement({vision.front_tile}) == WorldElement::Body) {
    event = Event::Collided;
    alive = false;
  } else if (GetWorldViewElement(vision.front_tile) == WorldElement::Food) {
    event = Event::Ate;
    // Increase snake's size and speed.
    size++;
    speed += 0.02;
  }

#if DEBUG_MODE
  std::cout << "Snake sensed front tile!" << std::endl;
#endif
}

void Snake::UpdateBodyAndWorldView(const SDL_Point& prev_head_position) {
#if DEBUG_MODE
  std::cout << "Snake began advancing world view..." << std::endl;
#endif

  // Perform matricial operation and advance world view matrix in one tile ahead.
  std::vector<Tensor> output;
  Scope local_root = Scope::NewRootScope();
  ClientSession local_session(local_root);
  //session.Run({advance}, &output);

  auto adv_operation = MatMul(local_root, advance_operator.GetTensor(), vision.world.GetTensor());

#if DEBUG_MODE
  std::cout << "Operation began running..." << std::endl;
#endif

  local_session.Run({adv_operation}, &output);
  
#if DEBUG_MODE
  std::cout << "Operation ran..." << std::endl;
#endif

  // Update vision matrix with the result of the operation.
  vision.world = std::move(output[0]);


  // Next update the current head position as a head in the snake world view.
  SetWorldViewElement(position.head, WorldElement::Head);

  // Update the snake body location, if it has one.
  if (size > 1) {
    // If the snake has a body, add previous head location to the body vector.
    position.body.push_back(prev_head_position);
    // Also update this position as a body part, in the world view.
    SetWorldViewElement(prev_head_position, Snake::WorldElement::Body);

    // Next, in case the snake didn't eat, move the oldest body vector item (i.e. the snake's tail).
    if (event != Snake::Event::Ate) {
      // Remove the tail from the snake vision of the world grid.
      SetWorldViewElement(position.body.front(), Snake::WorldElement::None);

      // Remove the tail from the body vector.
      position.body.erase(position.body.begin());
    }
  } else {
    // Otherwise, if the snake has only a head, clear the previous head position in the world view.
    SetWorldViewElement(prev_head_position, Snake::WorldElement::None);
  }

#if DEBUG_MODE
  std::cout << "Snake advanced world view!" << std::endl;
#endif
}

void Snake::TurnEyes() {
#if DEBUG_MODE
  std::cout << "Snake began turning eyes..." << std::endl;
#endif

  std::vector<Tensor> output;
  Scope local_root = Scope::NewRootScope();
  ClientSession local_session(local_root);

  if(action == Action::MoveLeft) {
    auto turn_operation = MatMul(local_root, turn_operator_b.GetTensor(), MatMul(local_root, vision.world.GetTensor(), turn_operator_a.GetTensor(), {true, false}));
    // Rotate snake vision grid 90 degrees to the right
    //session.Run({turn_left}, &output);
    local_session.Run({turn_operation}, &output);
  } else {
    auto turn_operation = MatMul(local_root, turn_operator_a.GetTensor(), MatMul(local_root, vision.world.GetTensor(), turn_operator_b.GetTensor(), {true, false}));
    // Rotate snake vision grid 90 degrees to the left
    //session.Run({turn_right}, &output);
    local_session.Run({turn_operation}, &output);
  }

  // Update vision matrix with the result of the operation.
  vision.world = std::move(output[0]);

#if DEBUG_MODE
  std::cout << "Snake turned eyes!" << std::endl;
#endif
}