#include "world.h"

World::World(const std::size_t& grid_width, const std::size_t& grid_height) : 
    snake(grid_width, grid_height),
    engine(dev()),
    random_w(0, static_cast<int>(grid_width - 1)),
    random_h(0, static_cast<int>(grid_height - 1)) {

}

void World::CreateSnake() {
    snake.Resurrect();
}

void World::GrowSnake() {
    
}

void World::KillSnake() {
    
}

void World::GrowFood() {
    int x, y;
    while (true) {
        x = random_w(engine);
        y = random_h(engine);
        // Check that the location is not occupied by a snake item before placing
        // food.
        if (!snake.SnakeCell(x, y)) {
            food.x = x;
            food.y = y;
            return;
        }
    }
}

Snake::Event World::MoveSnake() {
    
}

void World::Update() {
  if (!snake.IsAlive()) return;

  snake.Update();

  // Check if there's food over here
  if (food.x == snake.head.x && food.y == snake.head.y) {
    GrowFood();
    // Grow snake and increase speed.
    snake.Eat();
  }

  // Make snake AI model learn, based on the result of its latest action.
  snake.Learn();

  // Update snake vision input, based on the current world state.
  snake.SeeWorld(food);

  // AI model defines the snake's next action.
  snake.DefineAction();
}