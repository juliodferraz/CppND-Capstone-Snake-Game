#include "world.h"
#include <cmath>
#include <algorithm>

World::World(const std::size_t& grid_side_size) :
    grid_side_size(grid_side_size),
    snake(grid_side_size),
    engine(dev()),
    random_w(0, static_cast<int>(grid_side_size - 1)),
    random_h(0, static_cast<int>(grid_side_size - 1)),
    grid(grid_side_size, grid_side_size) {
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
  cellGrid.clear();

  // Initialize the cell grid.
  cellGrid.insert(cellGrid.begin, grid_side_size, std::vector<std::shared_ptr<Cell>>(grid_side_size, std::make_shared<Cell>()));
  for (int row = 0; row < grid_side_size; row++) {
    for (int col = 0; col < grid_side_size; col++) {
      cellGrid[row][col]->openPathsCount = 4;

      // Up direction
      if (row > 0) {
        cellGrid[row][col]->paths[Snake::Direction::Up] = cellGrid[row-1][col]->paths[Snake::Direction::Down];
      } else {
        cellGrid[row][col]->paths[Snake::Direction::Up] = std::make_shared<Path>();
      }
      cellGrid[row][col]->paths[Snake::Direction::Up]->cells[1] = cellGrid[row][col];

      // Down direction
      cellGrid[row][col]->paths[Snake::Direction::Down] = std::make_shared<Path>();
      cellGrid[row][col]->paths[Snake::Direction::Down]->cells[0] = cellGrid[row][col];

      // Left direction
      if (col > 0) {
        cellGrid[row][col]->paths[Snake::Direction::Left] = cellGrid[row][col-1]->paths[Snake::Direction::Right];
      } else {
        cellGrid[row][col]->paths[Snake::Direction::Left] = std::make_shared<Path>();
      }
      cellGrid[row][col]->paths[Snake::Direction::Left]->cells[1] = cellGrid[row][col];

      // Right direction
      cellGrid[row][col]->paths[Snake::Direction::Right] = std::make_shared<Path>();
      cellGrid[row][col]->paths[Snake::Direction::Right]->cells[0] = cellGrid[row][col];
    }
  }

  // Initialize snake head tile.
  SetElement(grid, snake.GetPosition().head, World::Element::SnakeHead);
  SetNeighborObstacle(snake.GetPosition().head);

  // Initialize snake body tiles.
  if (snake.GetSize() > 1) {
    for(const SDL_Point& body_part : snake.GetPosition().body) {
      SetElement(grid, body_part, World::Element::SnakeBody);
      SetNeighborObstacle(body_part);
    }
    SetElement(grid, snake.GetTailPosition(), World::Element::SnakeTail);
  }
  
  // Initialize the world wall at the borders of the grid.
  for (int x = 0, y = 0; x < grid_side_size; x++) {
    SetElement(grid, {x,y}, Element::Wall);
    SetNeighborObstacle({x,y});
  }
  for (int x = 0, y = grid_side_size - 1; x < grid_side_size; x++) {
    SetElement(grid, {x,y}, Element::Wall);
    SetNeighborObstacle({x,y});
  }
  for (int x = 0, y = 0; y < grid_side_size; y++) {
    SetElement(grid, {x,y}, Element::Wall);
    SetNeighborObstacle({x,y});
  }
  for (int x = grid_side_size - 1, y = 0; y < grid_side_size; y++) {
    SetElement(grid, {x,y}, Element::Wall);
    SetNeighborObstacle({x,y});
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
    // TODO: change the algorithm to select from an unordered_map containing all available positions in the grid.
    new_food_position.x = random_w(engine);
    new_food_position.y = random_h(engine);
    // Place the food only in an available (non-occupied) location in the grid.
    if (GetElement(grid, new_food_position) == Element::None) {
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
  Snake::Direction prev_tail_direction{snake.GetTailDirection()};
  
  snake.Move();

  // Check if snake head moved of tile
  SDL_Point head_position{snake.GetPosition().head};

  if (head_position.x != prev_head_position.x || head_position.y != prev_head_position.y) {

    // Checks the new tile content and raises appropriate event (e.g. eating, collision, etc.)
    if (IsObstacle(head_position)) {
      snake.SetEvent(Snake::Event::Collided);

    } else {
      if (IsWall(GetAdjacentPosition(head_position, snake.GetDirection()))) snakeWallTouchpoints++;

      if (GetElement(grid, head_position) == Element::Food) {
        snake.SetEvent(Snake::Event::Ate);
      } else {
        snake.SetEvent(Snake::Event::NewTile);
      }

      // If the snake has a body, update the old head position to contain a snake body part
      if (snake.GetSize() > 1) {
        SetElement(grid, prev_head_position, Element::SnakeBody);
        //SetNeighborObstacle(prev_head_position);
      }

      // Move the snake head in the world grid to its new position
      SetElement(grid, head_position, Element::SnakeHead);
      SetNeighborObstacle(head_position);

      if (snake.GetEvent() == Snake::Event::Ate) {
        // Now that the food has been eaten, make new food appear in a free grid tile.
        GrowFood();
      } else {
        // Remove the previous tail position from the world grid, as the snake didn't grow.
        SetElement(grid, prev_tail_position, Element::None);
        RemoveNeighborObstacle(prev_tail_position);

        if (IsWall(GetAdjacentPosition(prev_tail_position, Snake::GetOppositeOf(prev_tail_direction)))) snakeWallTouchpoints--;
      }

      // Update the snake body
      snake.UpdateBody(prev_head_position);

      // Set the snake tail element in the world grid.
      if (snake.GetSize() > 1) SetElement(grid, snake.GetTailPosition(), Element::SnakeTail);

      if (snake.IsAutoModeOn()) {
        // In case snake autonomous mode is on.
        // Run algorithm for the next direction, by checking the snake's head surrroundings and suggesting a next direction.
        // Start from current direction evaluation.
        Snake::Direction direction = snake.GetDirection();
        SDL_Point nextPosition = GetAdjacentPosition(head_position, direction);
        bool collision = IsObstacle(nextPosition);
        int distanceToFood = DistanceToFood(nextPosition);
        bool forbiddenPosition = IsForbiddenPosition(nextPosition, direction);

        // Evaluate direction to the left of current one.
        // TODO: transform the two repeatec blocks of logic below into function calls.
        Snake::Direction candidateDirection = snake.GetLeftOf(snake.GetDirection());
        SDL_Point candidatePosition = GetAdjacentPosition(head_position, candidateDirection);
        bool candidateCollision = IsObstacle(candidatePosition);
        int candidateDistanceToFood = DistanceToFood(candidatePosition);
        bool candidateForbiddenPosition = IsForbiddenPosition(candidatePosition, candidateDirection);
        if (candidateCollision == false) {
          if (collision == true
              || forbiddenPosition == true
              || (candidateDistanceToFood < distanceToFood
                  && candidateForbiddenPosition == false)) {
            direction = candidateDirection;
            nextPosition = candidatePosition;
            collision = candidateCollision;
            distanceToFood = candidateDistanceToFood;
            forbiddenPosition = candidateForbiddenPosition;
          }
        }

        // Evaluate direction to the right of current one.
        candidateDirection = snake.GetRightOf(snake.GetDirection());
        candidatePosition = GetAdjacentPosition(head_position, candidateDirection);
        candidateCollision = IsObstacle(candidatePosition);
        candidateDistanceToFood = DistanceToFood(candidatePosition);
        candidateForbiddenPosition = IsForbiddenPosition(candidatePosition, candidateDirection);
        if (candidateCollision == false) {
          if (collision == true
              || forbiddenPosition == true
              || (candidateDistanceToFood < distanceToFood
                  && candidateForbiddenPosition == false)) {
            direction = candidateDirection;
            nextPosition = candidatePosition;
            collision = candidateCollision;
            distanceToFood = candidateDistanceToFood;
            forbiddenPosition = candidateForbiddenPosition;
          }
        }

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
    case Element::Wall:
      return true;
    default:
      return false;
  }
}

bool World::IsSnakePart(const SDL_Point& position) const {
  switch (GetElement(grid, position)) {
    case Element::SnakeBody:
    case Element::SnakeTail:
      return true;
    default:
      return false;
  }
}

bool World::IsWall(const SDL_Point& position) const {
  switch (GetElement(grid, position)) {
    case Element::Wall:
      return true;
    default:
      return false;
  }
}

bool World::IsForbiddenPosition(const SDL_Point& position, const Snake::Direction& direction) const {
  if (IsObstacle(position) || IsDeadEnd(position)) return true;
  else {
    if ((IsSnakePart(GetAdjacentPosition(position, direction))
            || (IsWall(GetAdjacentPosition(position, direction))
                && snakeWallTouchpoints > 0))
          && !IsObstacle(GetAdjacentPosition(position, Snake::GetLeftOf(direction)))
          && !IsObstacle(GetAdjacentPosition(position, Snake::GetRightOf(direction)))) {
      return true;
    }
    else return false;
  }
}

void World::SetNeighborObstacle(const SDL_Point& position) {
  SDL_Point adjacentUp = GetAdjacentPosition(position, Snake::Direction::Up);
  SetValue(neighborObstaclesGrid, adjacentUp, GetValue(neighborObstaclesGrid, adjacentUp) + 1);

  SDL_Point adjacentRight = GetAdjacentPosition(position, Snake::Direction::Right);
  SetValue(neighborObstaclesGrid, adjacentRight, GetValue(neighborObstaclesGrid, adjacentRight) + 1);

  SDL_Point adjacentDown = GetAdjacentPosition(position, Snake::Direction::Down);
  SetValue(neighborObstaclesGrid, adjacentDown, GetValue(neighborObstaclesGrid, adjacentDown) + 1);

  SDL_Point adjacentLeft = GetAdjacentPosition(position, Snake::Direction::Left);
  SetValue(neighborObstaclesGrid, adjacentLeft, GetValue(neighborObstaclesGrid, adjacentLeft) + 1);
  
  if (IsObstacle(adjacentUp) == false
      && GetValue(neighborObstaclesGrid, adjacentUp) > 2) SetDeadEnd(adjacentUp);
  if (IsObstacle(adjacentRight) == false
      && GetValue(neighborObstaclesGrid, adjacentRight) > 2) SetDeadEnd(adjacentRight);
  if (IsObstacle(adjacentDown) == false
      && GetValue(neighborObstaclesGrid, adjacentDown) > 2) SetDeadEnd(adjacentDown);
  if (IsObstacle(adjacentLeft) == false
      && GetValue(neighborObstaclesGrid, adjacentLeft) > 2) SetDeadEnd(adjacentLeft);
}

void World::RemoveNeighborObstacle(const SDL_Point& position) {
  SDL_Point adjacentUp = GetAdjacentPosition(position, Snake::Direction::Up);
  SetValue(neighborObstaclesGrid, adjacentUp, GetValue(neighborObstaclesGrid, adjacentUp) - 1);

  SDL_Point adjacentRight = GetAdjacentPosition(position, Snake::Direction::Right);
  SetValue(neighborObstaclesGrid, adjacentRight, GetValue(neighborObstaclesGrid, adjacentRight) - 1);

  SDL_Point adjacentDown = GetAdjacentPosition(position, Snake::Direction::Down);
  SetValue(neighborObstaclesGrid, adjacentDown, GetValue(neighborObstaclesGrid, adjacentDown) - 1);

  SDL_Point adjacentLeft = GetAdjacentPosition(position, Snake::Direction::Left);
  SetValue(neighborObstaclesGrid, adjacentLeft, GetValue(neighborObstaclesGrid, adjacentLeft) - 1);

  if (IsObstacle(adjacentUp) == false
      && GetValue(neighborObstaclesGrid, adjacentUp) < 3) RemoveDeadEnd(adjacentUp);
  if (IsObstacle(adjacentRight) == false
      && GetValue(neighborObstaclesGrid, adjacentRight) < 3) RemoveDeadEnd(adjacentRight);
  if (IsObstacle(adjacentDown) == false
      && GetValue(neighborObstaclesGrid, adjacentDown) < 3) RemoveDeadEnd(adjacentDown);
  if (IsObstacle(adjacentLeft) == false
      && GetValue(neighborObstaclesGrid, adjacentLeft) < 3) RemoveDeadEnd(adjacentLeft);

  if (GetValue(neighborObstaclesGrid, position) > 2) SetDeadEnd(position);
}

void World::SetDeadEnd(const SDL_Point& position) {
  if (IsDeadEnd(position)) return;
  else {
    SetValue(deadEndGrid, position, 1);

    SDL_Point adjacent = GetAdjacentPosition(position, Snake::Direction::Up);
    if (IsObstacle(adjacent) == false
        && GetValue(neighborObstaclesGrid, adjacent) > 1) SetDeadEnd(adjacent);
    
    adjacent = GetAdjacentPosition(position, Snake::Direction::Right);
    if (IsObstacle(adjacent) == false
        && GetValue(neighborObstaclesGrid, adjacent) > 1) SetDeadEnd(adjacent);
    
    adjacent = GetAdjacentPosition(position, Snake::Direction::Down);
    if (IsObstacle(adjacent) == false
        && GetValue(neighborObstaclesGrid, adjacent) > 1) SetDeadEnd(adjacent);

    adjacent = GetAdjacentPosition(position, Snake::Direction::Left);
    if (IsObstacle(adjacent) == false
        && GetValue(neighborObstaclesGrid, adjacent) > 1) SetDeadEnd(adjacent);
  }
}

void World::RemoveDeadEnd(const SDL_Point& position) {
  if (!IsDeadEnd(position)) return;
  else {
    SetValue(deadEndGrid, position, 0);
    
    if (GetValue(neighborObstaclesGrid, position) == 2) {
      SDL_Point adjacent = GetAdjacentPosition(position, Snake::Direction::Up);
      if (IsObstacle(adjacent) == false
          && GetValue(neighborObstaclesGrid, adjacent) < 3) RemoveDeadEndRecursive(adjacent);
      
      adjacent = GetAdjacentPosition(position, Snake::Direction::Right);
      if (IsObstacle(adjacent) == false
          && GetValue(neighborObstaclesGrid, adjacent) < 3) RemoveDeadEndRecursive(adjacent);
      
      adjacent = GetAdjacentPosition(position, Snake::Direction::Down);
      if (IsObstacle(adjacent) == false
          && GetValue(neighborObstaclesGrid, adjacent) < 3) RemoveDeadEndRecursive(adjacent);

      adjacent = GetAdjacentPosition(position, Snake::Direction::Left);
      if (IsObstacle(adjacent) == false
          && GetValue(neighborObstaclesGrid, adjacent) < 3) RemoveDeadEndRecursive(adjacent);
    }
  }
}

void World::RemoveDeadEndRecursive(const SDL_Point& position) {
  if (!IsDeadEnd(position)) return;
  else {
    SetValue(deadEndGrid, position, 0);

    SDL_Point adjacent = GetAdjacentPosition(position, Snake::Direction::Up);
    if (IsObstacle(adjacent) == false
        && GetValue(neighborObstaclesGrid, adjacent) < 3) RemoveDeadEndRecursive(adjacent);
    
    adjacent = GetAdjacentPosition(position, Snake::Direction::Right);
    if (IsObstacle(adjacent) == false
        && GetValue(neighborObstaclesGrid, adjacent) < 3) RemoveDeadEndRecursive(adjacent);
    
    adjacent = GetAdjacentPosition(position, Snake::Direction::Down);
    if (IsObstacle(adjacent) == false
        && GetValue(neighborObstaclesGrid, adjacent) < 3) RemoveDeadEndRecursive(adjacent);

    adjacent = GetAdjacentPosition(position, Snake::Direction::Left);
    if (IsObstacle(adjacent) == false
        && GetValue(neighborObstaclesGrid, adjacent) < 3) RemoveDeadEndRecursive(adjacent);
  }
}

bool World::IsDeadEnd(const SDL_Point& position) const {
  if (GetValue(deadEndGrid, position) > 0) return true;
  else return false;
}