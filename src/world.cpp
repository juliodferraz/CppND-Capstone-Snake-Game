#include "world.h"
#include <cmath>
#include <algorithm>

World::World(const std::size_t& grid_side_size) :
    grid_side_size(grid_side_size),
    snake(grid_side_size),
    engine(dev()),
    random_w(0, static_cast<int>(grid_side_size - 1)),
    random_h(0, static_cast<int>(grid_side_size - 1)),
    grid(grid_side_size, grid_side_size),
    objectiveGrid(grid_side_size, grid_side_size) {
  // Initialize the snake.
  Reset();

  // Set the objective grid values, based on food and obstacles positions.
  //ResetObjectiveGrid();
  //SetValue(objectiveGrid, food, 0);
  //SetObjectiveGrid(food);

  #if DEBUG_MODE
    std::cout << "World object created" << std::endl;
  #endif
}

void World::Reset() {
  // Initialize snake and its world perception.
  snake.Init();

  // TODO: remove the world grid initialization below from this call, due to not being related to snake initialization?
  // Initialize world grid/map based on the snake location.
  InitWorldGrid();

  // Initialize the food.
  GrowFood();
}

void World::InitWorldGrid() {
  // Initialize the snake's world view based on the food and its body positions.
  // Clear the current world grid elements.
  grid.Reset();

  // Initialize snake head tile.
  SetElement(grid, snake.GetPosition().head, World::Element::SnakeHead);

  // Initialize snake body tiles.
  for(const SDL_Point& body_part : snake.GetPosition().body) {
    SetElement(grid, body_part, World::Element::SnakeBody);
  }

  // Initialize the world wall at the borders of the grid.
  for (int x = 0, y = 0; x < grid_side_size; x++) {
    SetElement(grid, {x,y}, Element::Wall);
  }
  for (int x = 0, y = grid_side_size - 1; x < grid_side_size; x++) {
    SetElement(grid, {x,y}, Element::Wall);
  }
  for (int x = 0, y = 0; y < grid_side_size; y++) {
    SetElement(grid, {x,y}, Element::Wall);
  }
  for (int x = grid_side_size - 1, y = 0; y < grid_side_size; y++) {
    SetElement(grid, {x,y}, Element::Wall);
  }

  #if DEBUG_MODE
    std::cout << "World grid initiated!" << std::endl;
  #endif
}

void World::GrowFood() {
  #if DEBUG_MODE
    std::cout << "Food began to grow..." << std::endl;
  #endif

  SDL_Point new_food_position;
  while (true) {
    new_food_position.x = random_w(engine);
    new_food_position.y = random_h(engine);
    // Place the food only in an available (non-occupied) location in the grid.
    if (IsObstacle(new_food_position) == false) {
      food = new_food_position;
      SetElement(grid, food, World::Element::Food);
      break;
    }
  }

  #if DEBUG_MODE
    std::cout << "Food grown!" << std::endl;
  #endif
}

void World::Update() {
  #if DEBUG_MODE
    std::cout << "World update begin" << std::endl;
  #endif

  // If the snake is deceased, no world update needs to be done.
  if (!snake.IsAlive()) return;

  // Otherwise, move the snake in its current direction.
  SDL_Point prev_head_position{snake.GetPosition().head};
  SDL_Point prev_tail_position{snake.GetTailPosition()};
  
  snake.Move();

  // Check if snake head moved of tile
  SDL_Point head_position{snake.GetPosition().head};

  if (head_position.x != prev_head_position.x || head_position.y != prev_head_position.y) {
    // Checks the new tile content and raises appropriate event (e.g. eating, collision, etc.)
    if (IsObstacle(head_position)) {
      snake.SetEvent(Snake::Event::Collided);

    } else {
      if (GetElement(grid, head_position) == Element::Food) {
        snake.SetEvent(Snake::Event::Ate);
      } else {
        snake.SetEvent(Snake::Event::NewTile);
      }

      // If the snake has a body, update the old head position to contain a snake body part
      if (snake.GetSize() > 1) SetElement(grid, prev_head_position, Element::SnakeBody);

      // Move the snake head in the world grid to its new position
      SetElement(grid, head_position, Element::SnakeHead);

      if (snake.GetEvent() == Snake::Event::Ate) {
        // Now that the food has been eaten, make new food appear in a free grid tile.
        GrowFood();
      } else {
        // Remove the previous tail position from the world grid, as the snake didn't grow.
        SetElement(grid, prev_tail_position, Element::None);
      }

      // Update the snake body
      snake.UpdateBody(prev_head_position);

      // Set the snake tail element in the world grid.
      if (snake.GetSize() > 1) SetElement(grid, snake.GetTailPosition(), Element::SnakeTail);

      // Set the objective grid values, based on food and obstacles positions.
      //ResetObjectiveGrid();
      //SetValue(objectiveGrid, food, 0);
      //SetObjectiveGrid(food);

      if (snake.IsAutoModeOn()) {
        // In case snake autonomous mode is on.
        // Run algorithm for the next direction, by checking the snake's head surrroundings and suggesting a next direction.
        // Start from current direction evaluation.
        Snake::Direction direction = snake.GetDirection();
        SDL_Point nextPosition = GetAdjacentPosition(head_position, direction);
        bool collision = IsObstacle(nextPosition);
        int distanceToFood = DistanceToFood(nextPosition); //GetValue(objectiveGrid, nextPosition);
        int neighborBodyCount = NeighborBodyCount(nextPosition);

        static int prevNeighborBodyCount = 0;

        // Evaluate direction to the left of current one.
        // TODO: transform the two repeatec blocks of logic below into function calls.
        Snake::Direction candidateDirection = snake.GetLeftOf(snake.GetDirection());
        SDL_Point candidatePosition = GetAdjacentPosition(head_position, candidateDirection);
        bool candidateCollision = IsObstacle(candidatePosition);
        int candidateDistanceToFood = DistanceToFood(candidatePosition); //GetValue(objectiveGrid, candidatePosition);
        int candidateNeighborBodyCount = NeighborBodyCount(candidatePosition);
        if (candidateCollision == false) {
          if (collision == true
              || (snake.GetObjective() == Snake::Objective::UniformBody
                  && (candidateNeighborBodyCount > neighborBodyCount
                      || (candidateNeighborBodyCount == neighborBodyCount
                          && candidateDistanceToFood < distanceToFood)))
              || (snake.GetObjective() == Snake::Objective::Food
                  && candidateDistanceToFood < distanceToFood)) {
            direction = candidateDirection;
            nextPosition = candidatePosition;
            collision = candidateCollision;
            distanceToFood = candidateDistanceToFood;
            neighborBodyCount = candidateNeighborBodyCount;
          }
        }

        // Evaluate direction to the right of current one.
        candidateDirection = snake.GetRightOf(snake.GetDirection());
        candidatePosition = GetAdjacentPosition(head_position, candidateDirection);
        candidateCollision = IsObstacle(candidatePosition);
        candidateDistanceToFood = DistanceToFood(candidatePosition); //GetValue(objectiveGrid, candidatePosition);
        candidateNeighborBodyCount = NeighborBodyCount(candidatePosition);
        if (candidateCollision == false) {
          if (collision == true
              || (snake.GetObjective() == Snake::Objective::UniformBody
                  && (candidateNeighborBodyCount > neighborBodyCount
                      || (candidateNeighborBodyCount == neighborBodyCount
                          && candidateDistanceToFood < distanceToFood)))
              || (snake.GetObjective() == Snake::Objective::Food
                  && candidateDistanceToFood < distanceToFood)) {
            direction = candidateDirection;
            nextPosition = candidatePosition;
            collision = candidateCollision;
            distanceToFood = candidateDistanceToFood;
            neighborBodyCount = candidateNeighborBodyCount;
          }
        }

        // In case the snake body is 100% uniform already (i.e. the next position adjacent body count is 0),
        // change the snake priority to be the proximity to the food, for the next decision.
        // This is necessary for the snake to not have the risk of being stuck in an infinite loop in certain
        // situations, without getting closer to the food.
        //if (neighborBodyCount == 0) snake.SetObjective(Snake::Objective::Food);
        if (neighborBodyCount == 0 
            && neighborBodyCount < prevNeighborBodyCount) snake.SetObjective(Snake::Objective::Food);
        else snake.SetObjective(Snake::Objective::UniformBody);

        prevNeighborBodyCount = neighborBodyCount;

        // Suggest direction with best evaluation to snake.
        snake.SetDirection(direction);
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

// Not used anywhere yet
inline World::Element World::GetElement(const Matrix& matrix, const SDL_Point& position) const {
  return static_cast<World::Element>(matrix.GetAt(position.y, position.x));
}

inline void World::SetElement(Matrix& matrix, const SDL_Point& position, const World::Element& new_element) {
  matrix(position.y, position.x) = static_cast<int>(new_element);
}

inline int World::GetValue(const Matrix& matrix, const SDL_Point& position) const {
  return matrix.GetAt(position.y, position.x);
}

inline void World::SetValue(Matrix& matrix, const SDL_Point& position, const int& new_value) {
  matrix(position.y, position.x) = new_value;
}

int World::DistanceToFood(const SDL_Point& position) const {
  // Calculate "city block" distance from snake head to food, considering only the distance inside the grid boundaries.
  return abs(position.x - food.x) + abs(position.y - food.y);
}

int World::NeighborBodyCount(const SDL_Point& position) const {
  // Count the number of spaces around a specific position in the grid containing Snake Body Parts.
  // Considers a world without walls.
  // Doesn't consider the snake tail for the counting.
  int count = 0;
  if (GetElement(grid, GetAdjacentPosition(position, Snake::Direction::Up)) == Element::SnakeBody) count++;
  if (GetElement(grid, GetAdjacentPosition(position, Snake::Direction::Right)) == Element::SnakeBody) count++;
  if (GetElement(grid, GetAdjacentPosition(position, Snake::Direction::Down)) == Element::SnakeBody) count++;
  if (GetElement(grid, GetAdjacentPosition(position, Snake::Direction::Left)) == Element::SnakeBody) count++;
  return count;
}

SDL_Point World::GetAdjacentPosition(const SDL_Point& position, const Snake::Direction& direction) const {
  // Considers a world without walls.
  switch (direction) {
    case Snake::Direction::Up:
      return {position.x, (int)fmod(position.y - 1 + grid_side_size, grid_side_size)};
      break;
    case Snake::Direction::Down:
      return {position.x, (int)fmod(position.y + 1 + grid_side_size, grid_side_size)};
      break;
    case Snake::Direction::Left:
      return {(int)fmod(position.x - 1 + grid_side_size, grid_side_size), position.y};
      break;
    default:
      return {(int)fmod(position.x + 1 + grid_side_size, grid_side_size), position.y};
      break;
  }
}

bool World::IsObstacle(const SDL_Point& position) const {
  switch (GetElement(grid, position)) {
    case Element::SnakeBody:
    case Element::SnakeTail:
    case Element::SnakeHead:
    case Element::Wall:
      return true;
    default:
      return false;
  }
}

void World::ResetObjectiveGrid() {
  for (int i = 0; i < grid_side_size; i++) {
    for (int j = 0; j < grid_side_size; j++) {
      objectiveGrid(i,j) = 0xFFFF;
    }
  }
  return;
}

void World::SetObjectiveGrid(const SDL_Point& reference) {
  // Starting from a reference position, iterate over adjacent grid positions, setting their value incrementally, to represent
  // the effective distance to the initial reference.
  // If reference position contains an obstacle (snake body part or wall), there's no need to proceed with the calculations.
  if (IsObstacle(reference)) return;

  if (GetValue(objectiveGrid, GetAdjacentPosition(reference, Snake::Direction::Up))
        > 1 + GetValue(objectiveGrid, reference)) {
    SetValue(objectiveGrid, GetAdjacentPosition(reference, Snake::Direction::Up), 1 + GetValue(objectiveGrid, reference));
    SetObjectiveGrid(GetAdjacentPosition(reference, Snake::Direction::Up));
  }
  if (GetValue(objectiveGrid, GetAdjacentPosition(reference, Snake::Direction::Down))
        > 1 + GetValue(objectiveGrid, reference)) {
    SetValue(objectiveGrid, GetAdjacentPosition(reference, Snake::Direction::Down), 1 + GetValue(objectiveGrid, reference));
    SetObjectiveGrid(GetAdjacentPosition(reference, Snake::Direction::Down));
  }
  if (GetValue(objectiveGrid, GetAdjacentPosition(reference, Snake::Direction::Left))
        > 1 + GetValue(objectiveGrid, reference)) {
    SetValue(objectiveGrid, GetAdjacentPosition(reference, Snake::Direction::Left), 1 + GetValue(objectiveGrid, reference));
    SetObjectiveGrid(GetAdjacentPosition(reference, Snake::Direction::Left));
  }
  if (GetValue(objectiveGrid, GetAdjacentPosition(reference, Snake::Direction::Right))
        > 1 + GetValue(objectiveGrid, reference)) {
    SetValue(objectiveGrid, GetAdjacentPosition(reference, Snake::Direction::Right), 1 + GetValue(objectiveGrid, reference));
    SetObjectiveGrid(GetAdjacentPosition(reference, Snake::Direction::Right));
  }
  return;
}