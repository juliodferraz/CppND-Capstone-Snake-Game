#include "world.h"
#include "clip.h"
#include <stdexcept>
#include <chrono>
#include <string>

World::World(const unsigned int gridSideLen) :
    gridSideLen(gridSideLen),
    randGenerator(std::chrono::system_clock::now().time_since_epoch().count()) {
  // Initialize the world grid and food.
  Init();
}

void World::Init() {
  // Initialize world grid/map.
  InitWorldGrid();

  // Initialize the food.
  if (GrowFood()) return;
  else throw std::runtime_error("World grid with no position available to initialize food.");
}

bool World::GrowFood() {
  // Place the food only in an available (non-occupied) location in the grid.
  if (!freeGridPositions.empty()) {
    // Select a random index from the free grid positions map.
    std::uniform_int_distribution<int> randomPosition{0, static_cast<int>(freeGridPositions.size()) - 1};
    int randIndex = randomPosition(randGenerator);
    std::unordered_map<World::Element*,SDL_Point>::iterator it = freeGridPositions.begin();
    std::advance(it,randIndex);

    // Initialize the food at the randomly selected empty grid spot.
    this->food = it->second;
    SetElement(food, Element::Food);

    return true;
  }
  else return false;
}

bool World::IsObstacle(const SDL_Point& position) const {
  // First check if position is inside the world grid boundaries. Otherwise, already return true.
  if (IsInsideBoundaries(position)) {
    // If inside grid boundaries, check if position is already filled with a collidable element. Return false otherwise.
    switch (GetElement(position)) {
      case Element::SnakeBody:
      case Element::SnakeHead:
      case Element::Wall:
        return true;
      default:
        return false;
    }
  } else return true;
}

void World::SetElement(const SDL_Point& position, const World::Element element) {
  // Check if position is inside grid boundaries first. If it isn't, raise a runtime exception.
  if (IsInsideBoundaries(position)) {
    grid[position.y][position.x] = element;
    if (element != Element::None) {
      auto searchResult = freeGridPositions.find(&grid[position.y][position.x]);
      if (searchResult != freeGridPositions.end()) freeGridPositions.erase(searchResult);
    } else {
      // If the position holds no element, make sure it is present in the free positions container.
      freeGridPositions.insert({&grid[position.y][position.x], position}); 
    }
  } else throw std::runtime_error("Out-of-boundaries world grid position (x = " + std::to_string(position.x) 
                                    + ", y = " + std::to_string(position.y) + ") trying to be set.");
}

World::Element World::GetElement(const SDL_Point& position) const { 
  if (IsInsideBoundaries(position)) {
    return grid[position.y][position.x];
  } else throw std::runtime_error("Out-of-boundaries world grid position (x = " + std::to_string(position.x) 
                                    + ", y = " + std::to_string(position.y) + ") trying to be read.");
}

void World::InitWorldGrid() {
  // Clear the current world grid elements.
  freeGridPositions.clear();
  grid.clear();

  // Initialize the world grid.
  grid.insert(grid.begin(), gridSideLen, std::vector<World::Element>(gridSideLen));
  
  // Initialize world grid cells as empty/free.
  for (int row = 0; row < gridSideLen; row++) {
    for (int col = 0; col < gridSideLen; col++) {
      grid[row][col] = World::Element::None;
      freeGridPositions[&grid[row][col]] = {col,row};
    }
  }

  // Initialize the world walls at the borders of the grid.
  for (int x = 0, y = 0; x < gridSideLen; x++) {
    SetElement({x,y}, Element::Wall);
  }
  for (int x = 0, y = gridSideLen - 1; x < gridSideLen; x++) {
    SetElement({x,y}, Element::Wall);
  }
  for (int x = 0, y = 0; y < gridSideLen; y++) {
    SetElement({x,y}, Element::Wall);
  }
  for (int x = gridSideLen - 1, y = 0; y < gridSideLen; y++) {
    SetElement({x,y}, Element::Wall);
  }
}

bool World::IsInsideBoundaries(const SDL_Point& position) const {
  if (position.x >= 0 && position.x < gridSideLen
      && position.y >= 0 && position.y < gridSideLen) return true;
  else return false; 
}