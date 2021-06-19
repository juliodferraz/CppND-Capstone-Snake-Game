#include "world.h"
#include <cmath>
#include <algorithm>

void Cell::SetContent(const World::ELement& element) {
  switch (element) {
    case World::Element::SnakeBody:
    case World::Element::Wall:
      this->free = false;
      for (int dir = 0; dir < 4; dir++) {
        this->paths[static_cast<Snake::Direction>(dir)]->open = false;
      }
      break;
    case World::Element::Food:
    case World::Element::SnakeHead:
    case World::Element::SnakeTail:
    case World::Element::None:
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
}

bool Cell::IsDeadend(const Snake::Direction& sourceDir) const {
  if (this->content == World::Element::SnakeTail) return false;
  else {
    int openPaths = 0;
    Snake::Direction dir = sourceDir;
    Snake::Direction openDir;
    for (int i = 0; i < 3; i++) {
      dir = Snake::GetRightOf(dir);
      if (this->paths[dir]->open) {
        openPaths++;
        openDir = dir;
      }
    }

    if (openPaths > 1) return false;
    else if (openPaths == 1) return this->paths[openDir]->GetDestinationCell(openDir)->IsDeadend(Snake::GetOppositeOf(openDir));
    else return true;
  }
}

World::World(const std::size_t& grid_side_size) :
    grid_side_size(grid_side_size),
    snake(grid_side_size),
    engine(dev()),
    random_w(0, static_cast<int>(grid_side_size - 1)),
    random_h(0, static_cast<int>(grid_side_size - 1)),
    grid(grid_side_size, grid_side_size) {
  // Initialize the snake.
  Reset();

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
  SetElement(snake.GetPosition().head, World::Element::SnakeHead);

  // Initialize snake body tiles.
  if (snake.GetSize() > 1) {
    for(const SDL_Point& body_part : snake.GetPosition().body) {
      SetElement(body_part, World::Element::SnakeBody);
    }
    SetElement(snake.GetTailPosition(), World::Element::SnakeTail);
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

  SDL_Point new_food_position;
  while (true) {
    // TODO: change the algorithm to select from an unordered_map containing all available positions in the grid.
    new_food_position.x = random_w(engine);
    new_food_position.y = random_h(engine);
    // Place the food only in an available (non-occupied) location in the grid.
    if (GetElement(new_food_position) == Element::None) {
      food = new_food_position;
      SetElement(food, World::Element::Food);
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
      if (GetElement(head_position) == Element::Food) {
        snake.SetEvent(Snake::Event::Ate);
      } else {
        snake.SetEvent(Snake::Event::NewTile);
      }

      // If the snake has a body, update the old head position to contain a snake body part
      if (snake.GetSize() > 1) {
        SetElement(prev_head_position, Element::SnakeBody);
      }

      // Move the snake head in the world grid to its new position
      SetElement(head_position, Element::SnakeHead);

      if (snake.GetEvent() == Snake::Event::Ate) {
        // Now that the food has been eaten, make new food appear in a free grid tile.
        GrowFood();
      } else {
        // Remove the previous tail position from the world grid, as the snake didn't grow.
        SetElement(prev_tail_position, Element::None);
      }

      // Update the snake body
      snake.UpdateBody(prev_head_position);

      // Set the snake tail element in the world grid.
      if (snake.GetSize() > 1) SetElement(snake.GetTailPosition(), Element::SnakeTail);

      if (snake.IsAutoModeOn()) {
        // In case snake autonomous mode is on.
        // Run algorithm for the next direction, by checking the snake's head surrroundings and suggesting a next direction.
        // Start from the left of current direction, then current direction, then right of it.
        Snake::Direction dir = Snake::GetLeftOf(snake.GetDirection());
        std::shared_ptr<Path> path = GetCell(head_position)->paths[dir];
        Snake::Direction bestDir = dir;
        bool bestDirOpen = path->open;
        bool bestDirDeadend = path->GetDestinationCell(dir)->IsDeadend(Snake::GetOppositeOf(dir));
        int bestFoodDistance = DistanceToFood(GetAdjacentPosition(head_position, dir));
        
        for (int i = 0; i < 2; i++) {
          dir = Snake::GetRightOf(dir);
          path = GetCell(head_position)->paths[dir];
          bool dirDeadend = path->GetDestinationCell(dir)->IsDeadend(Snake::GetOppositeOf(dir));
          int foodDistance = DistanceToFood(GetAdjacentPosition(head_position, dir));
          if (openDir == false
              || (path->open == true && bestDirDeadend == true)
              || (path->open == true && dirDeadend == false && foodDistance < bestFoodDistance)) {
            bestDirection = dir;
            bestDirOpen = path->open;
            bestDirDeadend = dirDeadend;
            bestFoodDistance = foodDistance;
          }
        }

        // Suggest direction with best evaluation to snake.
        snake.SetDirection(bestDir);
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
inline World::Element World::GetElement(const SDL_Point& position) const {
  return static_cast<World::Element>(grid.GetAt(position.y, position.x));
}

inline std::shared_ptr<Cell> GetCell(const SDL_Point& position) const {
  return cellGrid[position.y][position.x];
}

inline void World::SetElement(const SDL_Point& position, const World::Element& new_element) {
  grid(position.y, position.x) = static_cast<int>(new_element);
  cellGrid[position.y][position.x]->SetContent(new_element);
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
  switch (GetElement(position)) {
    case Element::SnakeBody:
    case Element::Wall:
      return true;
    default:
      return false;
  }
}
