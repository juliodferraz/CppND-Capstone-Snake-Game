#ifndef GAME_H
#define GAME_H

#include "controller.h"
#include "renderer.h"
#include "world.h"
#include "snake.h"

/**
 *  \brief Class responsible for the arbitration of the game states and mechanics.
 */
class Game {
 public:
  /**
   *  \brief Constructor of Game class object.
   *  \param winWidth Width of the game window, in screen coordinates.
   *  \param winHeight Height of the game window, in screen coordinates.
   *  \param gridSideLen Length of the game grid side, in game coordinates.
   */
  Game(const unsigned int winWidth, const unsigned int winHeight, const unsigned int gridSideLen);

  /**
   *  \brief Runs the Snake game until it is over.
   *  \param targetFramePeriod Target game frame period, in milliseconds.
   *  \param timeLimitInFrames Game round time limit, in frame count, after which the snake dies automatically (e.g. simulating hunger).
   */
  void Run(const unsigned int targetFramePeriod, const unsigned int timeLimitInFrames);

  /**
   *  \brief Returns the maximum score achieved, considering all game rounds for the current execution.
   *  \return Maximum score achieved, in points.
   */
  unsigned int GetMaxScore() const { return maxScore; }

 private:
  /**
   *  \brief Returns the current game score (i.e. snake size increase).
   *  \return Current game score.
   */
  unsigned int GetScore() const { return (unsigned int) (snake.GetSize() - 1); }

  /**
   *  \brief Process a user command (or no command at all, i.e. UserCommand::None) and updates the game state accordingly.
   *  \param command Latest command issued by the player.
   */
  void UpdateState(const Controller::UserCommand command = Controller::UserCommand::None);

  /**
   *  \brief Resets the game to a starting state, in order for it to be replayed.
   */
  void Reset();

  /**
   *  \brief Writes the game state to a text file called "save_state.txt" in the game folder, allowing it to be resumed in the next
   * game execution.
   */
  void StoreSaveFile() const;

  /**
   *  \brief Tries to load the game state from a text file called "save_state.txt", in the game folder.
   */
  void LoadSaveFile();

  /**
   *  \brief Controller object, used for reading player inputs.
   */
  Controller controller;

  /**
   *  \brief Renderer object, used for rendering the game on screen.
   */
  Renderer renderer;

  /**
   *  \brief World object, encapsulating the game scenario and non-playable objects (e.g. the food) states.
   */
  World world;

  /**
   *  \brief Snake object, encapsulating the playable snake state.
   */
  Snake snake;

  /**
   *  \brief Flag indicating if the game is still running (true), or is over (false).
   */
  bool running{false};

  /**
   *  \brief Flag indicating if the game frame rate is being controlled to be maintained at a target (true), or if this control
   * is inactive (false).
   */
  bool fpsCtrlActv{false};

  /**
   *  \brief Frame count for the purpose of game round time limit tracking. The game automatically ends when this count achieves its
   * configured limit.
   */
  unsigned int timeLimitFrameCnt{0};

  /**
   *  \brief Maximum game score achieved, after all previous game rounds.
   */
  unsigned int maxScore{0};

  /**
   *  \brief Flag indicating if the player has won in the game or not.
   */
  bool victory{false};
};

#endif