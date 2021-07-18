#include "genalg.h"
#include <algorithm>
#include <cmath>

GenAlg::GenAlg(unsigned int genLength, unsigned int populationSize, unsigned int selectionSize, float mutationFactor)
  : genLength(std::max(genLength, (unsigned int) 1)), 
    populationSize(std::max(populationSize, (unsigned int) 1)), 
    selectionSize(std::max(std::min(selectionSize, populationSize), (unsigned int) 1)),
    mutationFactor(mutationFactor),
    engine(dev()), 
    random_uindex(0, this->selectionSize-1) {
    // Initialize tensor population.
    for(int i = 0; i < populationSize; i++) {
        std::pair<VectorXf,float> individual;
        individual.first = VectorXf::Random(genLength); // TODO: change to a random function with normal dist.
        individual.second = 0;

        population.push_back(individual);
    }
}

void GenAlg::GradeCurFitness(const float& fitness) {
    curIndividual->second = fitness;

    // Move on to next individual
    curIndividual++;
    // If the previous one was the last one from the population, proceed to new generation.
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
        GenerateOffspring();
    }

    // Set current individual to first member of new population.
    curIndividual = population.begin();
}

void GenAlg::GenerateOffspring() {
    // Select two individuals at random among the fittest. Use uniform distribution.
    // Also, same individual can be selected twice, to simplify calculation.
    unsigned int parentA = random_uindex(engine);
    unsigned int parentB = random_uindex(engine);
    std::pair<VectorXf,float> offspring;
    offspring.first = std::move(Crossover(population[parentA].first, population[parentB].first));
    offspring.second = 0;
    population.push_back(std::move(offspring));
}

VectorXf GenAlg::Crossover(const VectorXf& a, const VectorXf& b) {
    VectorXf result;

    // Generates a vector with random elements from a uniform distribution in range [0,1).
    float rand_cross_idx = (VectorXf::Random(1)(0) + 1)/2; // Random() returns number between [-1,1].
    unsigned int crossIndex = (unsigned int) std::round(rand_cross_idx * (float) genLength);

    // Set and return a Tensor containing Tensor A contents up to crossIndex, and Tensor B contents after that.
    // For each gen, also consider a small probability of a mutation occurring.
    VectorXf crossResult = VectorXf(genLength);
    if (crossIndex > 0) crossResult.segment(0,crossIndex) = a.segment(0,crossIndex);
    if (crossIndex < genLength) {
        crossResult.segment(crossIndex,genLength-crossIndex) = b.segment(crossIndex,genLength-crossIndex);
    }

    // The generated values will have mean 0 and standard deviation 1.
    VectorXf rand_normal_vec = VectorXf::Random(genLength); // TODO: change to a random function with normal dist.

    // Generates a vector with random elements from a uniform distribution in range [0,1).
    VectorXf rand_uniform_vec = (VectorXf::Random(genLength).array() + (float) 1) / (float) 2;
    Eigen::Array<bool,Eigen::Dynamic,1> mut_bool = rand_uniform_vec.array() < mutationFactor;
    Eigen::ArrayXf mut_offset = rand_normal_vec.array() * mut_bool.cast<float>();
    result = crossResult + mut_offset.matrix();
    
    return result;
}