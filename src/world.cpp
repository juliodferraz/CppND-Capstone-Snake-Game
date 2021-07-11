#include "world.h"
#include <cmath>
#include <algorithm>

void Cell::SetContent(const Element& element) {
  #if DEBUG_MODE
    std::cout << "Setting cell content..." << std::endl;
  #endif

  switch (element) {
    case Element::SnakeBody:
    case Element::Wall:
      this->free = false;
      for (int dir = 0; dir < 4; dir++) {
        this->paths[static_cast<Snake::Direction>(dir)]->open = false;
      }
      break;
    case Element::Food:
    case Element::SnakeHead:
    case Element::SnakeTail:
    case Element::None:
      //TODO: protect against cases where there isn't a neighbor (null pointer as destination in paths).
      this->free = true;
      this->paths[Snake::Direction::Up]->open = this->paths[Snake::Direction::Up]->cells[0]->free;
      this->paths[Snake::Direction::Right]->open = this->paths[Snake::Direction::Right]->cells[1]->free;
      this->paths[Snake::Direction::Down]->open = this->paths[Snake::Direction::Down]->cells[1]->free;
      this->paths[Snake::Direction::Left]->open = this->paths[Snake::Direction::Left]->cells[0]->free;
      break;
    default:
      break;
  }

  this->content = element;

  #if DEBUG_MODE
    std::cout << "Set cell content." << std::endl;
  #endif
}

//TODO: update input to be the direction of the snake, and get the opposite dir inside this function (instead of having to
// always get the opposite during the function call).
bool Cell::IsDeadend(const Snake::Direction& sourceDir, const int& searchLimit, 
                      const std::unordered_set<std::shared_ptr<Path>>& track) const {
  #if DEBUG_MODE
    std::cout << "Checking if cell is deadend..." << std::endl;
  #endif

  bool deadend = true;

  if (searchLimit < 0) deadend = false;
  else if (this->content == Element::SnakeTail) deadend = false;
  else {
    Snake::Direction dir = sourceDir;
    for (int i = 0; i < 3 && deadend == true; i++) {
      dir = Snake::GetRightOf(dir);
      if (this->paths.at(dir)->open
          && track.find(this->paths.at(dir)) == track.end()) {
        std::unordered_set<std::shared_ptr<Path>> updatedTrack = track; 
        updatedTrack.insert(this->paths.at(dir));
        deadend = deadend 
                  && this->paths.at(dir)->GetDestinationCell(dir)->IsDeadend(Snake::GetOppositeOf(dir), searchLimit-1, updatedTrack);
      }
    }
  }

  #if DEBUG_MODE
    std::cout << "Cell is deadend = " << deadend << std::endl;
  #endif

  return deadend;
}

World::World(const std::size_t& grid_side_size) :
    grid_side_size(grid_side_size),
    snake(SDL_Point{(int) grid_side_size/2, (int) grid_side_size/2}),
    engine(dev()),
    grid(grid_side_size, grid_side_size) {
  // Initialize the snake.
  Reset();

  #if DEBUG_MODE
    std::cout << "World object created" << std::endl;
  #endif
}

void World::Reset() {
  // Initialize snake and its world perception.
  snake.Init(SDL_Point{(int) grid_side_size/2, (int) grid_side_size/2});

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
  freeGridPositions.clear();

  #if DEBUG_MODE
    std::cout << "Grids cleared..." << std::endl;
  #endif

  // Initialize the cell grid.
  cellGrid.insert(cellGrid.begin(), grid_side_size, std::vector<std::shared_ptr<Cell>>(grid_side_size));
  
  #if DEBUG_MODE
    std::cout << "Initializing cell grid..." << std::endl;
  #endif
  
  for (int row = 0; row < grid_side_size; row++) {
    for (int col = 0; col < grid_side_size; col++) {
      cellGrid[row][col] = std::make_shared<Cell>();
      cellGrid[row][col]->position = {col,row};

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

      freeGridPositions.insert(cellGrid[row][col]);
    }
  }

  #if DEBUG_MODE
    std::cout << "Cell grid initialized..." << std::endl;
  #endif

  // Initialize snake head tile.
  SetElement(snake.GetTargetHeadPosition(), Element::SnakeHead);

  // Initialize snake body tiles.
  for(auto it = snake.GetPositionQueue().begin()+1; it != snake.GetPositionQueue().end(); it++) {
    SetElement(*it, Element::SnakeBody);
  }

  // Initialize the world wall at the borders of the grid.
  for (int x = 0, y = 0; x < grid_side_size; x++) {
    SetElement({x,y}, Element::Wall);
  }
  for (int x = 0, y = grid_side_size - 1; x < grid_side_size; x++) {
    SetElement({x,y}, Element::Wall);
  }
  for (int x = 0, y = 0; y < grid_side_size; y++) {
    SetElement({x,y}, Element::Wall);
  }
  for (int x = grid_side_size - 1, y = 0; y < grid_side_size; y++) {
    SetElement({x,y}, Element::Wall);
  }

  #if DEBUG_MODE
    std::cout << "World grid initiated!" << std::endl;
  #endif
}

void World::GrowFood() {
  #if DEBUG_MODE
    std::cout << "Food began to grow..." << std::endl;
  #endif

  // Place the food only in an available (non-occupied) location in the grid.
  if (!freeGridPositions.empty()) {
    std::uniform_int_distribution<int> random_position{0, static_cast<int>(freeGridPositions.size()) - 1};
    int randIndex = random_position(engine);
    std::unordered_set<std::shared_ptr<Cell>>::iterator it = freeGridPositions.begin();
    for (int i = 0; i < randIndex; i++) it++;
    food = (*it)->position;
    SetElement(food, Element::Food);
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
  snake.Move();

  // Check if snake head is about to move to a new tile.
  SDL_Point targetHeadPosition{snake.GetTargetHeadPosition()};
  SDL_Point headPosition{snake.GetHeadPosition()};
  SDL_Point tailPosition{snake.GetTailPosition()};

  if (!(targetHeadPosition == headPosition)) {
    // Checks the new tile content and raises appropriate event (e.g. eating, collision, etc.)
    if (IsObstacle(targetHeadPosition)) {
      snake.SetEvent(Snake::Event::Collided);

    } else {
      if (GetElement(targetHeadPosition) == Element::Food) {
        snake.SetEvent(Snake::Event::Ate);
      } else {
        snake.SetEvent(Snake::Event::NewTile);
      }

      // If the snake has a body, update the current head position to contain a snake body part
      if (snake.GetSize() > 1) SetElement(headPosition, Element::SnakeBody);

      // Move the snake head in the world grid to its new position
      SetElement(targetHeadPosition, Element::SnakeHead);

      if (snake.GetEvent() == Snake::Event::Ate) {
        // Now that the food has been eaten, make new food appear in a free grid tile.
        GrowFood();
      } else {
        // Remove the previous tail position from the world grid, as the snake didn't grow.
        SetElement(tailPosition, Element::None);
      }

      // Set the snake tail element in the world grid.
      if (snake.GetSize() > 1) SetElement(snake.GetTailPosition(), Element::SnakeTail);

      if (snake.IsAutoModeOn()) {
        // In case snake autonomous mode is on.
        // Run algorithm for the next direction, by checking the snake's head surrroundings and suggesting a next direction.
        // Start from current direction evaluation.
        Snake::Direction direction = snake.GetDirection();
        SDL_Point nextPosition = GetAdjacentPosition(targetHeadPosition, direction);
        bool collision = IsObstacle(nextPosition);
        int distanceToFood = DistanceToFood(nextPosition);
        int neighborBodyCount = NeighborBodyCount(nextPosition);

        // Evaluate direction to the left of current one.
        // TODO: transform the two repeatec blocks of logic below into function calls.
        Snake::Direction candidateDirection = snake.GetLeftOf(snake.GetDirection());
        SDL_Point candidatePosition = GetAdjacentPosition(targetHeadPosition, candidateDirection);
        bool candidateCollision = IsObstacle(candidatePosition);
        int candidateDistanceToFood = DistanceToFood(candidatePosition);
        int candidateNeighborBodyCount = NeighborBodyCount(candidatePosition);
        if (candidateCollision == false) {
          if (collision == true
              || candidateNeighborBodyCount > neighborBodyCount
              || (candidateNeighborBodyCount == neighborBodyCount
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
        candidatePosition = GetAdjacentPosition(targetHeadPosition, candidateDirection);
        candidateCollision = IsObstacle(candidatePosition);
        candidateDistanceToFood = DistanceToFood(candidatePosition);
        candidateNeighborBodyCount = NeighborBodyCount(candidatePosition);
        if (candidateCollision == false) {
          if (collision == true
              || candidateNeighborBodyCount > neighborBodyCount
              || (candidateNeighborBodyCount == neighborBodyCount
                  && candidateDistanceToFood < distanceToFood)) {
            direction = candidateDirection;
            nextPosition = candidatePosition;
            collision = candidateCollision;
            distanceToFood = candidateDistanceToFood;
            neighborBodyCount = candidateNeighborBodyCount;
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
inline Element World::GetElement(const SDL_Point& position) const {
  return static_cast<Element>(grid.GetAt(position.y, position.x));
}

inline void World::SetElement(const SDL_Point& position, const Element& new_element) {
  grid(position.y, position.x) = static_cast<int>(new_element);
  cellGrid[position.y][position.x]->SetContent(new_element);
  if (new_element != Element::None) {
    auto searchResult = freeGridPositions.find(cellGrid[position.y][position.x]);
    if (searchResult != freeGridPositions.end()) freeGridPositions.erase(searchResult);
  } else {
    freeGridPositions.insert(cellGrid[position.y][position.x]); // The position is only inserted in case it isn't already present in the set.
  }
}

int World::DistanceToFood(const SDL_Point& position) const {
  // Calculate "city block" distance from snake head to food, considering only the distance inside the grid boundaries.
  return GetManhattanDistance(position, food);
}

int World::NeighborBodyCount(const SDL_Point& position) const {
  // Count the number of spaces around a specific position in the grid containing Snake Body Parts.
  // Considers a world without walls.
  // Doesn't consider the snake tail for the counting.
  int count = 0;
  if (GetElement(GetAdjacentPosition(position, Snake::Direction::Up)) == Element::SnakeBody) count++;
  if (GetElement(GetAdjacentPosition(position, Snake::Direction::Right)) == Element::SnakeBody) count++;
  if (GetElement(GetAdjacentPosition(position, Snake::Direction::Down)) == Element::SnakeBody) count++;
  if (GetElement(GetAdjacentPosition(position, Snake::Direction::Left)) == Element::SnakeBody) count++;
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
  switch (GetElement(position)) {
    case Element::SnakeBody:
    case Element::SnakeTail:
    case Element::SnakeHead:
    case Element::Wall:
      return true;
    default:
      return false;
  }
}