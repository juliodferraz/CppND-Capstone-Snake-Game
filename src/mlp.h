#ifndef MLP_H
#define MLP_H

#include <fstream>

#include <Eigen/Dense>

using Eigen::MatrixXf;
using Eigen::VectorXf; // Obs.: Eigen::VectorXf is a column vector by default.
using Eigen::Map;

/**
 *  \brief Class representing a Multi-Layer Perceptron (MLP), composed of layers of weights, inputs and outputs.
 */
class MLP {
 public:
  /**
   *  \brief Constructor of the MLP class object.
   *  \param inputSize Length of the input that the MLP accepts.
   *  \param layerSizes Vector indicating the length of each MLP layer, where each element corresponds to one layer, ordered from 
   * the first (non-input) layer to the output layer.
   */
  MLP(const unsigned int inputSize, const std::vector<unsigned int>& layerSizes);

  /**
   *  \brief Processes an input through the MLP and returns the resulting output vector.
   * The activation function for hidden layers neurons is the hyperbolic tangent (sigmoid with output in [-1;1] range).
   * And the activation function for output layer neurons is the logistic function (sigmoid with output in [0;1] range).
   * Note: if the input vector size doesn't match the number of MLP inputs, a runtime exception is thrown.
   *  \param input Input vector. Needs to be of the same input size expected by the MLP object, otherwise result will be random.
   *  \return Resulting output vector.
   */
  VectorXf GetOutput(VectorXf input);

  /**
   *  \brief Returns all the weights that form the MLP in vector format, ordered from the first to the last weight of each layer, 
   * from the first to the last layer.
   *  \return All MLP weights, from all layers, in vector format.
   */
  VectorXf GetWeightsVector();

  /**
   *  \brief Returns the total number of weights that form the MLP, considering all layers.
   *  \return Number of weights.
   */
  unsigned int GetWeightsCount() const { return weightsCnt; }

  /**
   *  \brief Sets the MLP weights to the input values.
   * If the input size doesn't match the number of weights in the MLP, a runtime exception is thrown.
   *  \param weights The weights to be set in the MLP, sequentially organized in a vector from the first to the last weight
   * in a layer, from the first to the last layer.
   */
  void SetWeights(const VectorXf& weights);

  /**
   *  \brief Stores the configuration of the MLP (input size, number of layers, and size of layers) in an output file stream.
   *  \param file Output file stream to which the MLP parameters will be written.
   */
  void StoreConfig(std::ofstream& file) const;

  /**
   *  \brief Loads the configuration of the MLP (input size, number of layers, and size of layers) from an input file stream.
   * Also sets the MLP object parameters to the read values and reinitializes the MLP weights to random values in the range [-1;1].
   *  \param file Input file stream from which the MLP parameters will be read.
   */
  void LoadConfig(std::ifstream& file);

  /**
   *  \brief Resets the MLP parameters to their default values (e.g. the number of layers and their sizes)
   * and reinitialize the MLP.
   */
  void Reset();

 private:
  /**
   *  \brief Clears the current MLP weights and reinitializes them to random values in the range [-1;1].
   */
  void Init();

  /**
   *  \brief Number of input dimensions in the MLP.
   */
  unsigned int inputSize;

  /**
   *  \brief Size of each MLP layer (number of neurons), from the first to the last (output) layer.
   */
  std::vector<unsigned int> layerSizes;

  /**
   *  \brief Default layer sizes vector.
   */
  const std::vector<unsigned int> defLayerSizes;

  /**
   *  \brief Total number of weights in the MLP (including the bias parameter of each neuron), considering all layers.
   */
  unsigned int weightsCnt;

  /**
   *  \brief The MLP weights, from the first to the last (output) layer. Each row represents the weights of a neuron (including
   * the bias weight). 
   * For each layer matrix, the number of rows represents the number of neurons in the layer; while the number of columns represents
   * the number of inputs to the layer.
   */
  std::vector<MatrixXf> layers;
};

#endif