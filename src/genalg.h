#ifndef GENALG_H
#define GENALG_H

#include <vector>
#include <random>
#include <utility>

#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/lib/gtl/array_slice.h"

#include "tensorflow/cc/ops/random_ops.h"

using namespace tensorflow;
using namespace tensorflow::ops;

// TODO: comment all
class GenAlg {
 public:
  GenAlg(unsigned int genLength, unsigned int populationSize, unsigned int selectionSize, float mutationFactor);
  const Tensor& GetCurIndividual() const;
  void GradeCurFitness(const float& fitness);

 private:
  void NewGeneration();
  void GenerateOffspring();
  Tensor Crossover(const Tensor& a, const Tensor& b);

  // Vector of pairs of weights sets and fitness value.
  std::vector<std::pair<Tensor,float>> population;
  std::vector<std::pair<Tensor,float>>::iterator curIndividual;
  unsigned int generation;

  unsigned int genLength;
  unsigned int populationSize;
  unsigned int selectionSize;

  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<unsigned int> random_uindex;

  Scope root;
  std::unique_ptr<ClientSession> session;
  Output parentA_var;
  Output parentB_var;
  Output cross_mut_op;
};

#endif