#include "genalg.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <chrono>
#include "clip.h"

GenAlg::GenAlg(const unsigned int chromLen, const unsigned int populationSize, 
                const unsigned int selectionSize, const float mutationFactor)
  : chromLen(std::max(chromLen, (unsigned int) 1)), 
    populationSize(std::max(populationSize, (unsigned int) 1)), 
    selectionSize(std::max(std::min(selectionSize, populationSize), (unsigned int) 1)),
    mutationFactor(mutationFactor),
    uniformIntDist(0, this->selectionSize-1),
    generator(std::chrono::system_clock::now().time_since_epoch().count()) {
  this->Init();
}

void GenAlg::Init() {
  // Resets the generation and individual count.
  this->generationCnt = 0;
  this->individualCnt = 0;

  // Clear previous population.
  population.clear();

  // Initialize tensor population from an uniform distribution in the range [-1;1].
  MatrixXf popMat = MatrixXf::Random(chromLen,populationSize);
  for(int i = 0; i < populationSize; i++) {
    std::pair<VectorXf,float> individual;
    individual.first = popMat.block(0, i, chromLen, 1);
    individual.second = 0;
    population.push_back(std::move(individual));
  }

  // Initialize current individual to first member of the population.
  curIndividual = population.begin();
}

void GenAlg::GradeCurFitness(const float& fitness) {
    // Set current individual fitness to input value.
    curIndividual->second = fitness;

    // Move genetic algorithm to next individual in the population.
    curIndividual++;
    individualCnt = CLPD_UINT_SUM(individualCnt, 1);

    // If all individuals have been evaluated and population ending has been reached, proceed to next generation.
    if (curIndividual == population.end()) NewGeneration();
}

void GenAlg::NewGeneration() {
    // Select the fittest members of the population.
    // Sort the population from most fittest to least.
    std::sort(population.begin(), population.end(), 
        [](const std::pair<VectorXf,float>& a, const std::pair<VectorXf,float>& b) { return a.second > b.second; });

    // Remove the least fittest individuals.
    population.erase(population.begin()+selectionSize, population.end());

    // Fill empty population spots with new offspring, using crossover and mutation operators.
    for (unsigned int i = selectionSize; i < populationSize; i++) {
        // Generate an offspring.
        // Select two individuals at random among the fittest. Use uniform distribution.
        // Also, same individual can be selected twice, to minimize computation.
        unsigned int parentA = uniformIntDist(generator);
        unsigned int parentB = uniformIntDist(generator);
        std::pair<VectorXf,float> offspring;
        offspring.first = std::move(Crossover(population[parentA].first, population[parentB].first));
        offspring.second = 0;
        population.push_back(std::move(offspring));
    }

    // Set current individual to first member of new population.
    curIndividual = population.begin();

    // Reset the individual count and increment the generation count.
    individualCnt = 0;
    generationCnt = CLPD_UINT_SUM(generationCnt, 1);
}

VectorXf GenAlg::Crossover(const VectorXf& a, const VectorXf& b) {
    VectorXf result;
    
    // Generates a vector with random elements from a uniform distribution in range [0,1).
    VectorXf randomProbVec = (VectorXf::Random(chromLen).array() + (float) 1) / (float) 2;
    // The random vector serves as the probability of selecting the first or second parent value for each position in the chromosome.
    Eigen::Array<bool,Eigen::Dynamic,1> parentAVec = randomProbVec.array() < (float) 0.5;
    Eigen::Array<bool,Eigen::Dynamic,1> parentBVec = !parentAVec;
    // Perform the crossover and generate the offspring.
    VectorXf crossResult = VectorXf(chromLen);
    crossResult = (parentAVec.cast<float>() * a.array() + parentBVec.cast<float>() * b.array()).matrix();

    // Next, execute the mutation operator.
    // Start with a random offset vector of mean 0 and standard deviation 1.
    VectorXf randomOffsetVec(chromLen);
    for (int i = 0; i < chromLen; i++) {
        randomOffsetVec[i] = normalDist(generator);
    }

    // Next, generate a probability vector with random elements from a uniform distribution in range [0,1).
    randomProbVec = (VectorXf::Random(chromLen).array() + (float) 1) / (float) 2;
    // The probability is used to define which chromosome positions shall be offset by the normal mutation operator.
    Eigen::Array<bool,Eigen::Dynamic,1> mutationEnableVec = randomProbVec.array() < mutationFactor;
    // Finally, add the mutation offset to the offspring.
    Eigen::ArrayXf mutationOffsetVec = randomOffsetVec.array() * mutationEnableVec.cast<float>();
    result = crossResult + mutationOffsetVec.matrix();

    return result;
}

void GenAlg::StoreState(std::ofstream& file) const {
  // Writes the genetic algorithm parameters to the argument file stream.
  file << chromLen << std::endl;
  file << populationSize << std::endl;
  file << selectionSize << std::endl;
  file << mutationFactor << std::endl;

  // Writes the genetic algorithm state to the argument file stream.
  file << generationCnt << std::endl;
  file << individualCnt << std::endl;
  for (int i = 0; i < populationSize; i++) {
    for (int j = 0; j < chromLen; j++) file << population[i].first[j] << " ";
    file << std::endl << population[i].second << std::endl;
  }
}

void GenAlg::LoadState(std::ifstream& file) {
  // Loads the genetic algorithm parameters from the argument file stream.
  file >> chromLen;
  file >> populationSize;
  file >> selectionSize;
  file >> mutationFactor;

  // Loads the genetic algorithm state from the argument file stream.
  file >> generationCnt;
  file >> individualCnt;
  population.clear();
  for (int i = 0; i < populationSize; i++) {
    population.push_back({VectorXf(chromLen), 0});
    for (int j = 0; j < chromLen; j++) file >> population[i].first[j];
    file >> population[i].second;
  }

  // Restore pointer to current individual being evaluated.
  curIndividual = population.begin() + individualCnt;
}