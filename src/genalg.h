#ifndef GENALG_H
#define GENALG_H

#include <vector>
#include <random>
#include <utility>

#include <Eigen/Dense>

using Eigen::MatrixXf;
using Eigen::VectorXf;

// TODO: comment all
class GenAlg {
 public:
  GenAlg(unsigned int genLength, unsigned int populationSize, unsigned int selectionSize, float mutationFactor);
  const VectorXf& GetCurIndividual() const { return curIndividual->first; }
  void GradeCurFitness(const float& fitness);

 private:
  void NewGeneration();
  void GenerateOffspring();
  VectorXf Crossover(const VectorXf& a, const VectorXf& b);

  // Vector of pairs of weights sets and fitness value.
  std::vector<std::pair<VectorXf,float>> population;
  std::vector<std::pair<VectorXf,float>>::iterator curIndividual;
  unsigned int generation;

  unsigned int genLength;
  unsigned int populationSize;
  unsigned int selectionSize;
  float mutationFactor;

  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<unsigned int> random_uindex;
};

#endif