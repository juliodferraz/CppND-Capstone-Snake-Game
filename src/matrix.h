#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>

//#include "tensorflow/cc/client/client_session.h"
//#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
//#include "tensorflow/core/public/session.h"
//#include "tensorflow/core/lib/gtl/array_slice.h"

using namespace tensorflow;
//using namespace tensorflow::ops;

// Signed int matrix (4 bytes per element, int32)
class Matrix {
 public:
  // Vector is initialized filled with zeros
  Matrix(const int& n_rows, const int& n_cols);

  // row and column are indexed from 0 to n_rows_/n_cols_ - 1.
  int& operator()(const int& row, const int& col);

  // Returns const reference (for when matrix needs only to be read, and not written).
  const int& GetAt(const int& row, const int& col) const;

  const Tensor& GetTensor() const { return tf_tensor_; }

  // Clears and resets all matrix elements to 0 (default value)
  // There is also some TF methods available that may be used for more efficiency: tensorflow::ops::Empty() and tensorflow::ops::Fill()
  Matrix& Reset(); //{ data_.assign(n_rows_*n_cols_, 0); }

  // TODO: comment
  //Matrix& operator=(const Tensor& tensor);
    // Copy data from input tensor. Is this the most efficient? Can the data be moved instead?
    //assert(tf_tensor_.CopyFrom(tensor, TensorShape({n_rows_,n_cols_})) == true);
  Matrix& operator=(Tensor&& tensor);

  // print operator
  friend std::ostream& operator<<(std::ostream& out, const Matrix& matrix) {
    auto data = matrix.tf_tensor_.tensor<int, 2>();
    for(int row = 0; row < matrix.n_rows_; row++) {
        out << "Row " << row << ": [";
        for(int col = 0; col < matrix.n_cols_; col++) {
          out << " " << data(row, col);
        }
        out << "]" << std::endl;
    }
    return out;
  }

 private:
  int n_rows_;
  int n_cols_;
  Tensor tf_tensor_;
};

#endif