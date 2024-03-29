#ifndef GAME_H
#define GAME_H

#include <unordered_map>

#include "controller.h"
#include "renderer.h"
#include "world.h"
#include "snake.h"
#include "coords2D.h"

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
   */
  void Run(const unsigned int targetFramePeriod);

  /**
   *  \brief Returns the maximum score achieved by the player.
   *  \return Maximum score achieved by player, in points.
   */
  unsigned int GetMaxScorePlayer() const { return maxScorePlayer; }

  /**
   *  \brief Returns the maximum score achieved by the AI.
   *  \return Maximum score achieved by the AI, in points.
   */
  unsigned int GetMaxScoreAI() const { return maxScoreAI; }

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
   *  \brief Starts a new game round while maintaining game record history and AI learning state.
   */
  void NewRound();

  /**
   *  \brief Completely resets the game and its data, clearing prior game record history and 
   * resetting AI learning state.
   */
  void ResetData();

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
   *  \brief Enumeration for possible ConfirmationBox user inputs.
   */
  enum class ConfirmationBoxButtonId { None, Yes, Cancel };

  /**
   *  \brief Opens a pop-up window that interrupts main thread execution and asks for an user input (Yes or Cancel).
   *  \param message String containing the message/question to be displayed.
   *  \param errMessage String containing the message to be displayed in case there's an error to SDL lib call.
   *  \param yesDefaultOption Flag indicating if the 'Yes' button shall be the default option.
   *  \return Option selected by the user.
   */
  ConfirmationBoxButtonId ShowConfirmationBox(const std::string& message, 
    const std::string& errMessage, const bool yesDefaultOption = false) const;

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
   *  \brief Flag indicating if the game has been paused by the player.
   */
  bool paused{false};

  /**
   *  \brief Flag indicating if the game frame rate is being controlled to be maintained at 60 fps target (true), 
   * or if this control is inactive (false).
   */
  bool fpsCtrlActv{true};

  /**
   *  \brief Maximum game score achieved by the player, after all previous game rounds.
   */
  unsigned int maxScorePlayer{0};

  /**
   *  \brief Maximum game score achieved by the AI in auto mode, after all previous game rounds.
   */
  unsigned int maxScoreAI{0};

  /**
   *  \brief Flag indicating if the player has won in the game or not.
   */
  bool victory{false};

  /**
   *  \brief Snake covered positions container.
   * This container maps key values of an Element address (representing a position in the game grid
   * the snake has covered) to a Direction2D representing the direction from which the snake entered
   * the same grid position.
   * This is used to identify the beginning of an endless loop of movement during auto (AI) mode, as
   * the MLP will provide the same output decision for the rest of the game round.
   * After the snake eats or a new game round starts, this container is emptied.
   * Then, after every grid position+direction set, this gets added to the container if not there
   * yet.
   * Otherwise, if the same position+direction is identified to be present in the map already, the 
   * snake is killed to end the game round as soon as possible, and thus accelerate the machine 
   * learning algorithm for the snake AI.
   */
  std::unordered_map<World::Element*, Direction2D> coveredPositions;
};

#endif