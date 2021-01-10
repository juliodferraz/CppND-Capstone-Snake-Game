#include "world.h"

World::World(const std::size_t& grid_side_size) :
    grid_side_size(grid_side_size),
    snake(grid_side_size),
    engine(dev()),
    random_w(0, static_cast<int>(grid_side_size - 1)),
    random_h(0, static_cast<int>(grid_side_size - 1)),
    grid(grid_side_size, grid_side_size) {
  // Initialize world grid as empty (No elements)
  grid.Reset();

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
  snake.Init(food);

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
      break;
    }
  }

  // Update new food position in snake world view, as well
  snake.SetWorldViewElement(new_food_position, Snake::WorldElement::Food);

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

  // Update the world map according to the result of the snake movement, if necessary.
  Snake::Event movementResult{snake.GetEvent()};
  if (movementResult == Snake::Event::NewTile || movementResult == Snake::Event::Ate) {
    // Move the snake head in the world grid to its new position
    SetElement(snake.GetPosition().head, World::Element::SnakeHead);
    // If the snake has a body, update the old head position to contain a snake body part
    if (snake.GetSize() > 1) SetElement(prev_head_position, World::Element::SnakeBody);

    if (movementResult == Snake::Event::NewTile) {
      // If the snake didn't eat, remove the previous tail position from the world grid,
      // because it didn't grow.
      SetElement(prev_tail_position, World::Element::None);
    } else {
      // Otherwise, in case the snake ate, make new food appear on a free grid tile.
      GrowFood();
    }
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