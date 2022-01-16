#include "snake.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <Eigen/Dense>
#include "clip.h"
#include "config.h"

using Eigen::VectorXf; // Column vector

Snake::Snake(const SDL_Point& startPosition, World& world) 
  : startPosition{startPosition},
    world{world},
    mlp(SNAKE_STIMULI_LEN, SNAKE_MLP_LAYERS_SIZES),
    genalg(mlp.GetWeightsCount(), GA_POPULATION_SIZE, GA_SURVIVORS_CNT, GA_MUTATION_RATE) {
  this->Init();
}

void Snake::Init() {
  // Initialize all snake object parameters.
  this->alive = true;
  this->event = Event::SameTile;
  this->action = Action::MoveFwd;
  this->direction = Direction2D::Up;
  this->forbiddenDir = Direction2D::Down;

  this->tarHeadPos = Coords2D(startPosition);
  this->positionQueue.clear();
  this->positionQueue.push_front(tarHeadPos);

  // Initialize snake head tile in world.
  this->world.SetElement(this->GetHeadPosition(), World::Element::SnakeHead);

  // Set MLP weights as the ones from the current individual in Genetic Algorithm population.
  this->mlp.SetWeights(genalg.GetCurIndividual());
}

void Snake::ProcessUserCommand(const Controller::UserCommand command) {
  if(command == Controller::UserCommand::ToggleAutoMode) ToggleAutoMode();
  else if(!automode) {
    // If auto mode is on, only the auto mode toggling command is available, and all other commands are ignored.
    // Else, check which command was received.
    switch(command) {
      case Controller::UserCommand::GoUp:
        if (forbiddenDir != Direction2D::Up) {
          // The command is only considered if the requested direction is different from the one pointing to the first snake body part.
          // This is done to avoid having the snake die by colliding with it's first body part while the head is still in the same 
          // grid tile, in case the player change directions too quickly.
          if (direction == Direction2D::Left) Act(Action::MoveRight);
          else if (direction == Direction2D::Right) Act(Action::MoveLeft);
          else Act(Action::MoveFwd);
        }
        break;
      case Controller::UserCommand::GoDown:
        if (forbiddenDir != Direction2D::Down) {
          if (direction == Direction2D::Left) Act(Action::MoveLeft);
          else if (direction == Direction2D::Right) Act(Action::MoveRight);
          else Act(Action::MoveFwd);
        }
        break;
      case Controller::UserCommand::GoLeft:
        if (forbiddenDir != Direction2D::Left) {
          if (direction == Direction2D::Up) Act(Action::MoveLeft);
          else if (direction == Direction2D::Down) Act(Action::MoveRight);
          else Act(Action::MoveFwd);
        }
        break;
      case Controller::UserCommand::GoRight:
        if (forbiddenDir != Direction2D::Right) {
          if (direction == Direction2D::Up) Act(Action::MoveRight);
          else if (direction == Direction2D::Down) Act(Action::MoveLeft);
          else Act(Action::MoveFwd);
        }
        break;
      default:
        // UserCommand::None (no command issued by user)
        break;
    }
  }
}

void Snake::Move() {
  switch (direction) {
    case Direction2D::Up:
      tarHeadPos += SDL_FPoint{0,-speed};
      break;

    case Direction2D::Down:
      tarHeadPos += SDL_FPoint{0,speed};
      break;

    case Direction2D::Left:
      tarHeadPos += SDL_FPoint{-speed,0};
      break;

    case Direction2D::Right:
      tarHeadPos += SDL_FPoint{speed,0};
      break;
  }
}

void Snake::SetEvent(const Event event) {
  this->event = event;

  // Update the snake position queue if needed, depending on the event.
  switch (this->event) {
    case Event::Killed:
      // If the snake collided or was directly killed for some other reason, it's now deceased.
      alive = false;
      break;
    case Event::NewTile:
      // Remove the previous tail position from the world grid, as the snake didn't grow.
      PopSnakeTailPos();
      // Set the current head position as the target one.
      PushNewSnakeHeadPos(tarHeadPos);
      // Set opposite to current direction as forbidden so that the snake cannot move to the same tile of its
      // first body part.
      UpdateForbiddenDir();
      break;
    case Event::Ate:
      // Set the current head position as the target one.
      PushNewSnakeHeadPos(tarHeadPos);
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
  /**
   * Build MLP input.
   * Input vector is composed of:
   * - distance to closest obstacle (wall or snake body) from the left side of the head;
   * - distance to closest obstacle from the front side of the head;
   * - distance to closest obstacle from the right side of the head;
   * - horizontal distance to the food from the front side of the head;
   * - vertical distance to the food from the front side of the head. 
   */
  VectorXf input(SNAKE_STIMULI_LEN);
  input[0] = GetDist2Obstacle(GetHeadPosition(), GetLeftOf(this->direction));
  input[1] = GetDist2Obstacle(GetHeadPosition(), this->direction);
  input[2] = GetDist2Obstacle(GetHeadPosition(), GetRightOf(this->direction));
  SDL_Point versor2Food = GetVersor(GetHeadPosition(), world.GetFoodPosition(), this->direction);
  input[3] = versor2Food.x;
  input[4] = versor2Food.y;

  // Run MLP and get output vector.
  VectorXf output = mlp.GetOutput(input);

  // Change or maintain direction depending on which output layer neuron presented the highest activation.
  // If neuron 0, move left; else if neuron 1, maintain direction; else if neuron 2, move right.
  if (output[0] > output[1]) {
    if (output[0] > output[2]) Act(Action::MoveLeft);
    else Act(Action::MoveRight);
  } else {
    if (output[2] > output[1]) Act(Action::MoveRight);
    else Act(Action::MoveFwd);
  }
}

void Snake::StoreState(std::ofstream& file) const {
  // Write the mlp configuration and genetic algorithm state.
  mlp.StoreConfig(file);
  genalg.StoreState(file);
}

void Snake::LoadState(std::ifstream& file) {
  mlp.LoadConfig(file);
  genalg.LoadState(file);

  // Reinitialize snake.
  this->Init();
}

void Snake::Act(const Action input) {
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

unsigned int Snake::GetDist2Obstacle(const SDL_Point& reference, const Direction2D direction) {
  SDL_Point adjPos = GetAdjPosition(reference, direction);
  if (world.IsObstacle(adjPos)) return 1;
  else return GetDist2Obstacle(adjPos, direction) + 1;
}