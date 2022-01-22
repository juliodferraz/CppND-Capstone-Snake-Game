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
#define SAVE_STATE_FILE_PATH "../save/save_state.txt"

/**
 *  Snake's AI MLP (Multi-layer Perceptron) and GA (Genetic Algorithm) parameters
 */

/**
 *  \brief Sequence of MLP layers sizes, where each element corresponds to one layer, ordered from 
 * the first (non-input) layer to the output layer.
 */
#define SNAKE_MLP_LAYERS_SIZES {5,5,3}

/**
 *  \brief Size of the Genetic Algorithm (GA) population at each generation, in number of individuals.
 */
#define GA_POPULATION_SIZE 1000

/**
 *  \brief Number/count of fittest individuals selected to survive and generate offspring, at each GA generation.
 */
#define GA_SURVIVORS_CNT 50

/**
 *  \brief Probability (between 0 and 1) of a gene mutation during crossover, independent for each gene.
 */
#define GA_MUTATION_RATE 0.02

#endif