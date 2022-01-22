#ifndef RENDERER_H
#define RENDERER_H

#include "SDL.h"
#include "world.h"

/**
 *  \brief Class responsible for the graphical rendering of the game.
 */
class Renderer {
 public:
  /**
   *  \brief Constructor of the Renderer class object.
   *  \param winWidth Width of the game window, in screen coordinates.
   *  \param winHeight Height of the game window, in screen coordinates.
   *  \param gridSideLen Length of the game grid side, in game coordinates.
   */
  Renderer(const unsigned int winWidth, const unsigned int winHeight, const unsigned int gridSideLen);
  
  /**
   *  \brief Destructor of the Renderer class object.
   */
  ~Renderer();

  /**
   *  \brief Renders current game screen state.
   *  \param world Reference to World object.
   */
  void Render(const World& world);

  /**
   *  \brief Updates information shown in the window title.
   *  \param score Current score.
   *  \param fps Current frames per second value.
   *  \param maxScorePlayer Maximum score that has been achieved by the player.
   *  \param automode Flag indicating if game is in auto mode.
   *  \param maxScoreAI Maximum score that has been achieved by the snake AI.
   *  \param genCnt Current genetic algorithm generation.
   *  \param indCnt Current genetic algorithm individual.
   *  \param gamePaused Indicates if game is paused.
   */
  void UpdateWindowTitle(const unsigned int score, const unsigned int fps, const unsigned int maxScorePlayer, 
    const bool automode, const unsigned int maxScoreAI, const unsigned int genCnt, 
    const unsigned int indCnt, const bool gamePaused);

 private:
  /**
   *  \brief The SDL library window object.
   */
  SDL_Window *sdlWindow;

  /**
   *  \brief The SDL library renderer object.
   */
  SDL_Renderer *sdlRenderer;

  /**
   *  \brief The width of the game screen window, in screen coordinates.
   */
  const unsigned int winWidth;

  /**
   *  \brief The height of the game screen window, in screen coordinates.
   */
  const unsigned int winHeight;

  /**
   *  \brief The side length of the square game grid, in game coordinates units.
   */
  const unsigned int gridSideLen;
};

#endif