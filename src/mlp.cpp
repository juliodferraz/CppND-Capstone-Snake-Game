#include "mlp.h"
#include <stdexcept>

MLP::MLP(const unsigned int inputSize, const std::vector<unsigned int>& layerSizes) : inputSize{inputSize}, layerSizes{layerSizes} {
    Init();
}

void MLP::Init() {
    // Clear previous layers weights.
    layers.clear();

    // Resets the total number of weights in the MLP.
    weightsCnt = 0;

    // Number of inputs to the first layer shall be the input size plus one for the bias input (which will always be equal to '1').
    unsigned int numCols = inputSize + 1;

    for(int i = 0; i < layerSizes.size(); i++) {
        // Initialize layer weights to random values between [-1;1].
        MatrixXf weights = MatrixXf::Random(layerSizes[i], numCols);
        layers.push_back(weights);

        // Add the current layer number of weights to the total number of weights in the MLP.
        weightsCnt += layerSizes[i] * numCols;

        // Number of inputs to the next layer shall be the prior layer size plus one for the bias input 
        // (which will always be equal to '1').
        numCols = layerSizes[i] + 1;
    }
}

VectorXf MLP::GetOutput(VectorXf input) {
    // Protect against the possibility of the function argument not having the correct size.
    // Its size should be equal to the total number of MLP inputs.
    if (input.size() != inputSize) {
        throw std::runtime_error("Error in MLP::GetOutput(VectorXf): input vector size doesn't match number of MLP inputs.");
    }

    // Add the bias value of 1 to the input vector.
    input.conservativeResize(inputSize + 1); // Resize while maintaining previous vector values.
    input[inputSize] = 1;

    // For the column vector input, process it in the MLP and return the output column vector.
    // For hidden layers, use hyperbolic tangent activation function (sigmoid with output in [-1;1] range) 
    // (in Eigen lib, tanh(A)).
    VectorXf layerOutput = input;
    for(int i = 0; i < layers.size()-1; i++) {
        layerOutput = (layers[i] * layerOutput).array().tanh();

        // Append bias value of 1 to the layer output
        layerOutput.conservativeResize(layerSizes[i] + 1);
        layerOutput[layerSizes[i]] = 1;
    }

    // For the output layer, use the logistic activation function (sigmoid with output in [0;1] range) 
    // (in Eigen lib, inverse(exp(-A)+1)).
    VectorXf mlpOutput;
    if(!layers.empty()) mlpOutput = ((-(layers.back() * layerOutput)).array().exp() + 1).inverse();
    else mlpOutput = input; // In case the MLP is empty (i.e. doesn't have any layers), output the same input vector.
    return mlpOutput;
}

VectorXf MLP::GetWeightsVector() {
    // Initialize a vector of size equal to the total number of weights in the MLP.
    VectorXf output(this->weightsCnt);

    int startIndex = 0;
    for(int i = 0; i < layers.size(); i++) {
        // For each layer, map the layers weights to a vector and write it to a segment of the vector that will be returned by this
        // function.
        Map<VectorXf> layerWeightsMap(layers[i].data(), layers[i].size());
        output.segment(startIndex, layers[i].size()) = layerWeightsMap;

        startIndex += layers[i].size();
    }

    return output;
}

void MLP::SetWeights(const VectorXf& weights) {
    // Protect against the possibility of the function argument not having the correct size.
    // Its size should be equal to the total number of weights in the MLP.
    if (weights.size() != this->weightsCnt) {
        throw std::runtime_error("Error in MLP::SetWeights(const VectorXf&): input vector size doesn't match number of MLP weights.");
    }

    int startIndex = 0;
    for(int i = 0; i < layers.size(); i++) {
        // For each layer, reads the next segment of the argument, map them to a matrix format
        // and sets them as the layer weights.
        VectorXf layerWeights = weights.segment(startIndex, layers[i].size());
        Map<MatrixXf> layerWeightsMap(layerWeights.data(), layers[i].rows(), layers[i].cols());
        layers[i] = layerWeightsMap;

        startIndex += layers[i].size();
    }
}

void MLP::StoreConfig(std::ofstream& file) const {
  // Write the MLP parameters to an output file stream.
  file << inputSize << std::endl;
  file << layerSizes.size() << std::endl;
  for (int i = 0; i < layerSizes.size(); i++) file << layerSizes[i] << " ";
  file << std::endl;
}

void MLP::LoadConfig(std::ifstream& file) {
  // Read the MLP parameters from the input stream, and update them.
  file >> inputSize;

  unsigned int layerCnt{0};
  file >> layerCnt;
  layerSizes = std::vector<unsigned int>(layerCnt);
  for (int i = 0; i < layerSizes.size(); i++) file >> layerSizes[i];

  // Also re-initializes the MLP weights.
  this->Init();
}