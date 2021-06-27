#include "genalg.h"
#include <algorithm>

GenAlg::GenAlg(unsigned int genLength, unsigned int populationSize, unsigned int selectionSize, float mutationFactor)
  : genLength(std::max(genLength, (unsigned int) 1)), populationSize(std::max(populationSize, (unsigned int) 1)), 
  selectionSize(std::max(std::min(selectionSize, populationSize), (unsigned int) 1)), 
  engine(dev()), random_uindex(0, this->selectionSize-1),
  root{Scope::NewRootScope()} {

    // Initialize Tensorflow session
    session = std::unique_ptr<ClientSession>(new ClientSession(root));

    // Building the crossover and mutation operator
    parentA_var = Placeholder(root, DT_FLOAT);
    parentB_var = Placeholder(root, DT_FLOAT);

    // Generates a vector with random elements from a uniform distribution in range [0,1).
    auto rand_cross_idx = RandomUniform(root, {1, 1}, DT_FLOAT);
    auto crossIndex = Round(root, Multiply(root, rand_cross_idx, {genLength}));

    // Set and return a Tensor containing Tensor A contents up to crossIndex, and Tensor B contents after that.
    // For each gen, also consider a small probability of a mutation occurring.
    auto crossResult = Concat(root, 
        {Slice(root, parentA_var, {0, 0}, Concat(root, {{1}, crossIndex}, 1)), 
        Slice(root, parentB_var, {0, crossIndex}, Concat(root, {{1}, Subtract(root, {genLength}, crossIndex)}, 1))}, 1);

    // The generated values will have mean 0 and standard deviation 1.
    auto rand_normal_vec = RandomNormal(root, {1, genLength}, DT_FLOAT);

    // Generates a vector with random elements from a uniform distribution in range [0,1).
    auto rand_uniform_vec = RandomUniform(root, {1, genLength}, DT_FLOAT);
    auto mut_thr = Fill(root, {1, genLength}, mutationFactor);
    auto mut_bool = Less(root, rand_uniform_vec, mut_thr);
    auto mut_idxs = Cast(root, mut_bool, DT_FLOAT);

    auto mut_offset = Multiply(root, rand_normal_vec, mut_idxs);

    cross_mut_op = Add(root, crossResult, mut_offset);

    // Initialize tensor population.
    auto init_pop_op = RandomNormal(root, {1, genLength}, DT_FLOAT);
    std::vector<Tensor> out_tensors;
    for(int i = 0; i < populationSize; i++) {
        TF_CHECK_OK(session->Run({init_pop_op}, &out_tensors));
        
        std::pair<Tensor,float> individual;
        individual.first = std::move(out_tensors[0]);
        individual.second = 0;

        population.push_back(std::move(individual));
    }
}

const Tensor& GenAlg::GetCurIndividual() const {
    return curIndividual->first;
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
        [](const std::pair<Tensor,float>& a, const std::pair<Tensor,float>& b) { return a.second > b.second; });

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
    std::pair<Tensor,float> offspring;
    offspring.first = std::move(Crossover(population[parentA].first, population[parentB].first));
    offspring.second = 0;
    population.push_back(std::move(offspring));
}

Tensor GenAlg::Crossover(const Tensor& a, const Tensor& b) {
    std::vector<Tensor> out_tensors;
    TF_CHECK_OK(session->Run({{parentA_var, a}, {parentB_var, b}}, {cross_mut_op}, &out_tensors));
    Tensor offspring = std::move(out_tensors[0]);

    return offspring;
}