#include "world.h"
#include <cmath>
#include <algorithm>
#include <iterator>

World::World(const std::size_t& grid_side_size) :
    grid_side_size(grid_side_size), engine(dev()) {
  // Initialize the snake.
  Reset();

  #if DEBUG_MODE
    std::cout << "World object created" << std::endl;
  #endif
}

void World::Reset() {
  // TODO: remove the world grid initialization below from this call, due to not being related to snake initialization?
  // Initialize world grid/map based on the snake location.
  InitWorldGrid();

  // Initialize the food.
  GrowFood();
}

void World::InsertSnake(const SDL_Point& head) { 
  positionQueue.clear();
  positionQueue.push_front(head);

  // Initialize snake head tile.
  SetElement(head, Element::SnakeHead);
}

void World::InitWorldGrid() {
  // Initialize the snake's world view based on the food and its body positions.
  // Clear the current world grid elements.
  freeGridPositions.clear();
  grid.clear();

  #if DEBUG_MODE
    std::cout << "Grids cleared..." << std::endl;
  #endif

  // Initialize the world grid.
  grid.insert(grid.begin(), grid_side_size, std::vector<World::Element>(grid_side_size));
  
  #if DEBUG_MODE
    std::cout << "Initializing cell grid..." << std::endl;
  #endif
  
  for (int row = 0; row < grid_side_size; row++) {
    for (int col = 0; col < grid_side_size; col++) {
      grid[row][col] = World::Element::None;
      freeGridPositions[&grid[row][col]] = {col,row};
    }
  }

  #if DEBUG_MODE
    std::cout << "Cell grid initialized..." << std::endl;
  #endif

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
    std::unordered_map<World::Element*,SDL_Point>::iterator it = freeGridPositions.begin();
    std::advance(it,randIndex);
    this->food = it->second;
    SetElement(food, Element::Food);
  }

  #if DEBUG_MODE
    std::cout << "Food grown!" << std::endl;
  #endif
}

void World::SetElement(const SDL_Point& position, const World::Element& new_element) {
  grid[position.y][position.x] = new_element;
  if (new_element != Element::None) {
    auto searchResult = freeGridPositions.find(&grid[position.y][position.x]);
    if (searchResult != freeGridPositions.end()) freeGridPositions.erase(searchResult);
  } else {
    freeGridPositions.insert({&grid[position.y][position.x], position}); 
    // The position is only inserted in case it isn't already present in the map.
  }
}

int World::DistanceToFood(const SDL_Point& position) const {
  // Calculate "city block" distance from snake head to food, considering only the distance inside the grid boundaries.
  return GetManhattanDistance(position, food);
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