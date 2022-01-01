#ifndef SNAKE_H
#define SNAKE_H

#include "controller.h"
#include "world.h"
#include "coords2D.h"
#include "genalg.h"
#include "MLP.h"

/**
 *  \brief Class managing the game's snake entity.
 */
class Snake {
 public:
  /**
   *  \brief Snake event enum, representing the possible outcomes of a movement.
   */
  enum class Event { SameTile, NewTile, Ate, Killed };

  /**
   *  \brief Snake action enum, representing the possible decisions of the snake AI model 
   * (i.e. move either forward, left or right of the current direction).
   */
  enum class Action { MoveFwd, MoveLeft, MoveRight };

  /**
   *  \brief Snake object constructor.
   *  \param startPosition The snake's starting position in the game grid.
   *  \param world Reference to the game world, so that it also can be changed based on snake's events.
   */
  Snake(const SDL_Point& startPosition, World& world);

  /**
   *  \brief Initializes the snake's parameters and world view.
   */
  void Init();

  /**
   *  \brief Updates the snake internal state based on the user command.
   *  \param command Latest command issued by the player.
   */
  void ProcessUserCommand(const Controller::UserCommand command);

  /**
   *  \brief Moves the snake (following its current direction), by updating its head location in the world.
   */
  void Move();

  /**
   *  \brief Sets the latest snake event, resulting from its last action, and updates other internal parameters based on the event.
   *  \param event The event to be set.
   */
  void SetEvent(const Event event);

  /**
   *  \brief Calculates the snake's AI model decision for the next snake action, based on the world state.
   */
  void DefineAction();

  /**
   *  \brief Stores the configuration and state of the Snake (more specifically, its MLP and Genetic Algorithm) in an output file stream.
   *  \param file Output file stream to which the Snake parameters will be written.
   */
  void StoreState(std::ofstream& file) const;

  /**
   *  \brief Loads the state of the Snake (more specifically, its MLP and Genetic Algorithm) from an input file stream, and re-initialize
   * the snake in the game grid.
   *  \param file Input file stream from which the Snake parameters will be read.
   */
  void LoadState(std::ifstream& file);

  /**
   *  \brief Returns the current snake action its AI model decided for.
   *  \return Current snake action.
   */
  Action GetAction() const { return action; }

  /**
   *  \brief Returns the latest snake event.
   *  \return Latest snake event.
   */
  Event GetEvent() const { return event; }

  /**
   *  \brief Returns the current snake direction.
   *  \return Current snake direction.
   */
  Direction2D GetDirection() const { return direction; }

  /**
   *  \brief Indicates if auto mode is on.
   *  \return True, if the snake is autonomous. False, if it's controllable by the player.
   */
  bool IsAutoModeOn() const { return automode; }

  /**
   *  \brief Indicates if snake is alive.
   *  \return True, if the snake is alive. False, if it is deceased.
   */
  bool IsAlive() const { return alive; }

  /**
   *  \brief Returns the current snake size.
   *  \return Current snake size.
   */
  int GetSize() const { return positionQueue.size(); }

  /**
   *  \brief Returns the current position of the snake's tail in the game grid. In case the snake's size is 1, returns the head position.
   *  \return The discrete coordinates of the snake's tail in the game grid (i.e. from player's perspective).
   */
  SDL_Point GetTailPosition() const { return positionQueue.back(); }

  /**
   *  \brief Returns the current position of the snake's head in the game grid.
   *  \return The discrete coordinates of the snake's head in the game grid (i.e. from player's perspective).
   */
  SDL_Point GetHeadPosition() const { return positionQueue.front(); }

  /**
   *  \brief Returns the current target position for the snake's head in the game grid. The actual head position may be different.
   *  \return The target coordinates of the snake's head in the game grid (i.e. from player's perspective).
   */
  SDL_Point GetTargetHeadPosition() const { return tarHeadPos; }

  /**
   *  \brief Returns the count of the Snake's genetic algorithm's generations.
   *  \return Unsigned int representing current generation number.
   */
  unsigned int GetGenAlgGeneration() const { return genalg.GetGenerationCnt(); }

  /**
   *  \brief Returns the count of already fitness-evaluated individuals in the current Snake's genetic algorithm generation.
   *  \return Unsigned int representing the index of the individual under fitness evaluation.
   */
  unsigned int GetGenAlgIndividual() const { return genalg.GetIndividualCnt(); }

  /**
   *  \brief Sets the fitness corresponding to the latest snake performance.
   * Obs.: this is only done in case the snake was controlled by CPU from start to end of the game, otherwise this learning
   * adaptation is skipped.
   *  \param fitness Value to be set as the fitness of the current individual in the snake's genetic algorithm, 
   * used to train/evolve the snake over time.
   */
  void GradeFitness(const float& fitness);
  
 private:

  /**
   *  \brief Makes the snake act.
   *  \param input Target action.
   */
  void Act(const Action input);

  /**
   *  \brief Removes the last element from the snake's position queue (i.e. its previous tail position) and also updates the same tile
   * in the game world grid.
   */
  void PopSnakeTailPos();

  /**
   *  \brief Pushes a new position as the first element of the snake's position queue (i.e. its head position) 
   * and also updates the prior head position tile in the game world grid accordingly.
   *  \param head The new snake's head position in the grid.
   */
  void PushNewSnakeHeadPos(const SDL_Point& head);

  /**
   *  \brief Calculates the distance from a reference point to the closest obstacle (wall or snake body part) in a specific direction.
   *  \param reference The reference grid position.
   *  \param direction The direction being considered.
   *  \return The absolute distance from the reference point to the closest obstacle in the input direction.
   */
  unsigned int GetDist2Obstacle(const SDL_Point& reference, const Direction2D direction);

  /**
   *  \brief Toggles the snake mode between auto (controlled by CPU) and manual (controllable by the player).
   * In case automode is disabled (i.e. player is now controlling the snake), also disables AI learning for the rest of the round.
   */
  void ToggleAutoMode();

  /**
   *  \brief Sets the direction opposite to the current snake one to be forbidden - meaning the player cannot change the snake's
   * direction to this one.
   * This is done to avoid having the snake die by colliding with it's first body part while the head is still in the same grid tile, 
   * in case the player change directions too quickly.
   */
  inline void UpdateForbiddenDir() { forbiddenDir = GetOppositeOf(direction); }

  /**
   *  \brief The current snake direction.
   */
  Direction2D direction{Direction2D::Up};

  /**
   *  \brief The current forbidden direction. The snake can move in any other direction except for this one.
   * This direction exists in order to avoid having the snake die by colliding with it's first body part while the head is still 
   * in the same grid tile, in case the player change directions too quickly.
   */
  Direction2D forbiddenDir{Direction2D::Down};

  /**
   *  \brief Double-ended queue containing the snake's head and body parts coordinates in the world. The double-ended queue
   * has constant complexity for push and pop operations at both queue ends, which makes it more efficient to be used here
   * instead of a vector (which displays linear complexity for operations at its front).
   */
  std::deque<SDL_Point> positionQueue;

  /**
   *  \brief The target position for the snake's head in the game grid. The actual head position may be different.
   */
  Coords2D tarHeadPos;

  /**
   *  \brief The starting position for the snake's head in the game grid, used during re-initializations of the snake.
   */
  SDL_Point startPosition;

  /**
   *  \brief Indicates snake's current life state (alive or deceased).
   */
  bool alive{true};

  /**
   *  \brief Current snake speed.
   */
  const float speed{0.2f};

  /**
   *  \brief The latest snake action.
   */
  Action action{Action::MoveFwd};

  /**
   *  \brief The latest snake event, as an outcome of its action.
   */
  Event event{Event::SameTile};

  /**
   *  \brief True, if the snake is autonomous and controlled by CPU. False, if it's controllable by the player.
   */
  bool automode{true};

  /**
   *  \brief Reference to the game world, so that it also can be changed based on snake's events.
   */
  World& world;

  /**
   *  \brief Multi-layer perceptron serving as the snake's AI decision model.
   */
  MLP mlp;

  /**
   *  \brief Genetic algorithm used to train the snake's MLP-based decision model over time.
   */
  GenAlg genalg;

  /**
   *  \brief True, if the snake is learning and current Genetic Algorithm individual fitness is under evaluation. 
   * False otherwise, in case the snake has been controlled by the player at any point in time during the current game.
   */
  bool learningMode{true};
};

#endif