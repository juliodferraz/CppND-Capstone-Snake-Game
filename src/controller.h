#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "snake.h"
#include <random>

class Controller {
 public:
  void HandleInput(bool &running, Snake &snake);

 private:
  void ChangeDirection(Snake &snake, Snake::Direction input,
                       Snake::Direction opposite) const;

  /**
   *  \brief Random number generator defining the snake direction changes during auto mode.
   */
  std::default_random_engine generator;

  /**
   *  \brief Uniform real distribution to be used during calculation of snake direction changes during auto mode.
   */
  std::uniform_real_distribution<float> random_direction_distribution{0.0, 1.0};
};

#endif