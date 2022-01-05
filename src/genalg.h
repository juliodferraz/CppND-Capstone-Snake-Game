#ifndef GENALG_H
#define GENALG_H

#include <vector>
#include <random>
#include <utility>
#include <fstream>

#include <Eigen/Dense>

using Eigen::MatrixXf;
using Eigen::VectorXf;

/**
 *  \brief Class responsible for running the genetic algorithm behind the snake's learning.
 */
class GenAlg {
 public:
  /**
   *  \brief Constructor of GenAlg class object.
   *  \param chromLen Size of an individual from the population, represented by a numerical chromosome string.
   *  \param populationSize Size of the population at each generation, in number of individuals.
   *  \param selectionSize Number of fittest individuals selected to survive and generate offspring, at each generation.
   *  \param mutationFactor Probability of a gene mutation during crossover, independent for each gene.
   */
  GenAlg(const unsigned int chromLen, const unsigned int populationSize, 
          const unsigned int selectionSize, const float mutationFactor);

  /**
   *  \brief Re-initializes the genetic algorithm from scratch, re-generating the population and resetting its state.
   */
  void Init();

  /**
   *  \brief Returns the individual/chromosome whose fitness is currently being evaluated.
   *  \return The individual/chromosome currently under evaluation, as a column vector.
   */
  const VectorXf& GetCurIndividual() const { return curIndividual->first; }

  /**
   *  \brief Sets the fitness of the individual/chromosome under current evaluation.
   *  \param fitness Floating-point value representing the fitness that shall be set for the individual.
   */
  void GradeCurFitness(const float& fitness);

  /**
   *  \brief Returns the current generation number.
   *  \return The current generation number, where 0 is the first generation.
   */
  unsigned int GetGenerationCnt() const { return generationCnt; }

  /**
   *  \brief Returns the current individual/chromosome under evaluation number.
   *  \return The current individual under evaluation, where 0 represents the first individual of the population.
   */
  unsigned int GetIndividualCnt() const { return individualCnt; }

  /**
   *  \brief Tries to store the current state of the genetic algorithm in a file, allowing the algorithm to be resumed at a later time.
   *  \param file Output file stream to which the algorithm state shall be written.
   */
  void StoreState(std::ofstream& file) const;

  /**
   *  \brief Tries to load a previous state of the genetic algorithm from a file, in order to resume it.
   *  \param file Input file stream from which the algorithm state shall be read.
   */
  void LoadState(std::ifstream& file);

 private:
  /**
   *  \brief Performs the natural selection algorithm, where only the fittest individuals survive, and then generates new offspring
   * to complete the population. This gives rise to a new generation of the genetic algorithm population.
   */
  void NewGeneration();

  /**
   *  \brief Performs a random crossover between the two input chromosomes, generating an offspring.
   *  \param a The first parent/crossover operand.
   *  \param b The second parent/crossover operand.
   *  \return The offspring vector. Each offspring gene is randomly selected between the respective parents genes at the same
   * position. Each gene in the offspring also has a chance that a random offset taken from a normal distribution with mean 0 and
   * stddev of 1 will be applied to it (mutation operand).
   */
  VectorXf Crossover(const VectorXf& a, const VectorXf& b);

  /**
   *  \brief Vector containing all chromosomes in the current generation population. Each element is a pair of a chromosome and its 
   * floating-point fitness value placeholder.
   */
  std::vector<std::pair<VectorXf,float>> population;

  /**
   *  \brief Iterator over the population, pointing to the current individual/chromosome whose fitness is being evaluated.
   */
  std::vector<std::pair<VectorXf,float>>::iterator curIndividual;

  /**
   *  \brief Current generation number, where 0 is the first generation.
   */
  unsigned int generationCnt{0};

  /**
   *  \brief Current individual under evaluation number, where 0 is the first individual in the population.
   */
  unsigned int individualCnt{0};

  /**
   *  \brief Length of the chromosomes in the population.
   */
  unsigned int chromLen;

  /**
   *  \brief Size of the population at each generation.
   */
  unsigned int populationSize;

  /**
   *  \brief Number of individuals that survive and generate offspring between consecutive generations.
   */
  unsigned int selectionSize;

  /**
   *  \brief Probability that an offspring gene will suffer a mutation during crossover (where a normal dist. offset would be added
   * to it).
   */
  float mutationFactor;

  /**
   *  \brief Random number generator. Initialized in class constructor with the system clock as a seed.
   */
  std::default_random_engine generator;

  /**
   *  \brief Uniform integer distribution for random numbers.
   */
  std::uniform_int_distribution<unsigned int> uniformIntDist;

  /**
   *  \brief Normal distribution for random numbers. With mean of 0 and standard deviation of 1.
   */
  std::normal_distribution<float> normalDist{0.0, 1.0};
};

#endif