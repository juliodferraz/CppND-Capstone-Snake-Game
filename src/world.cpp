#include "world.h"
#include <cmath>

World::World(const std::size_t& grid_side_size) :
    grid_side_size(grid_side_size),
    snake(grid_side_size, 32, 32),
    engine(dev()),
    random_w(0, static_cast<int>(grid_side_size - 1)),
    random_h(0, static_cast<int>(grid_side_size - 1)),
    grid(grid_side_size, grid_side_size) {
  // Initialize the food.
  GrowFood();

  // Initialize the snake.
  InitSnake();

  #if DEBUG_MODE
    std::cout << "World object created" << std::endl;
  #endif
}

void World::InitSnake() {
  // Initialize snake and its world perception.
  snake.Init();

  // TODO: remove the world grid initialization below from this call, due to not being related to snake initialization?
  // Initialize world grid/map based on the snake location.
  InitWorldGrid();
}

void World::InitWorldGrid() {
  // Initialize the snake's world view based on the food and its body positions.
  // Clear the current world grid elements.
  grid.Reset();

  // Initialize food tile.
  SetElement(food, World::Element::Food);

  // Initialize snake head tile.
  SetElement(snake.GetPosition().head, World::Element::SnakeHead);

  // Initialize snake body tiles.
  for(const SDL_Point& body_part : snake.GetPosition().body) {
    SetElement(body_part, World::Element::SnakeBody);
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
      if (snake.GetSize() > 1) SetElement(prev_head_position, Element::SnakeBody);

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
        // Start from current direction evaluation.
        Snake::Direction direction = snake.GetDirection();
        SDL_Point nextPosition = GetAdjacentPosition(head_position, direction);
        bool collision = IsObstacle(nextPosition);
        int distanceToFood = DistanceToFood(nextPosition);
        int neighborBodyCount = NeighborBodyCount(nextPosition);

        // Evaluate direction to the left of current one.
        // TODO: transform the two repeatec blocks of logic below into function calls.
        Snake::Direction candidateDirection = snake.GetLeftOf(snake.GetDirection());
        SDL_Point candidatePosition = GetAdjacentPosition(head_position, candidateDirection);
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
        candidatePosition = GetAdjacentPosition(head_position, candidateDirection);
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
inline World::Element World::GetElement(const SDL_Point& position) const {
  return static_cast<World::Element>(grid.GetAt(position.y, position.x));
}

inline void World::SetElement(const SDL_Point& position, const World::Element& new_element) {
  grid(position.y, position.x) = static_cast<int>(new_element);
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