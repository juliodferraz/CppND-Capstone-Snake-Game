#ifndef CONFIG_H
#define CONFIG_H
/**
 *  File used for centralizing all compile-time configurations of the game.
 */

/**
 *  \brief Standard target game frame rate, in frames per second.
 */
#define FRAME_RATE_FPS 60

/**
 *  \brief Standard target game frame period, in milliseconds.
 */
#define FRAME_PERIOD_MS (1000 / FRAME_RATE_FPS) 

/**
 *  \brief The width of the game screen window, in screen coordinates.
 */
#define WINDOW_WIDTH (GRID_SIDE_LENGTH * 21)

/**
 *  \brief The height of the game screen window, in screen coordinates.
 */
#define WINDOW_HEIGHT WINDOW_WIDTH

/**
 *  \brief The side length of the square game grid, in game coordinates units.
 */
#define GRID_SIDE_LENGTH 31

/**
 *  \brief The path and name of the file used to store the game history information and genetic algorithm state.
 */
#define SAVE_STATE_FILE_PATH "../save_state.txt"

// TODO: add parameters of MLP and genetic algorithm here

#endif