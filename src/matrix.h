#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/lib/gtl/array_slice.h"

using namespace tensorflow;
using namespace tensorflow::ops;

// A simple TensorBuffer implementation that allows us to create Tensors that
// -do not- take ownership of pre-allocated memory.
class MatrixBuffer : public TensorBuffer {
 public:
  // size value unit is bytes.
  MatrixBuffer(const void* ptr, const size_t& size_in_bytes)
      : TensorBuffer(const_cast<void*>(ptr)), size_(size_in_bytes) {}

  ~MatrixBuffer() override {
    // Decrements reference count by one, as reference from current object is going to be destructed.
    //this->Unref();
  }

  size_t size() const override { return size_; }

  TensorBuffer* root_buffer() override { return this; }

  void FillAllocationDescription(AllocationDescription* proto) const override {
    proto->set_allocated_bytes(size_);
  }

  /// \brief Whether this TensorBuffer owns the underlying memory.
  bool OwnsMemory() const override { return true; } // TODO confirm this

 private:
  size_t size_;
};

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
  Matrix& Reset(); //{ data_.assign(n_rows_*n_cols_, 0); }

  // TODO: comment
  //Matrix& operator=(const Tensor& tensor);
    // Copy data from input tensor. Is this the most efficient? Can the data be moved instead?
    //assert(tf_tensor_.CopyFrom(tensor, TensorShape({n_rows_,n_cols_})) == true);
  Matrix& operator=(Tensor&& tensor);

  // print operator
  friend std::ostream& operator<<(std::ostream& out, const Matrix& matrix) {
    /*
    int offset{0};
    for(int row = 0; row < matrix.n_rows_; row++) {
        out << "Row " << row << ": [";
        for(int col = 0; col < matrix.n_cols_; col++) {
          out << " " << matrix.tf_buffer_->base<int>()[offset];
          offset++;
        }
        out << "]" << std::endl;
    }
    return out;
    */
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
  //std::vector<int> data_; // TODO: REMOVE this - Looks like this variable is not needed anymore, as it seems that its contents are copied to tensor buffer during initialization

  Tensor tf_tensor_;
  MatrixBuffer* tf_buffer_;
};

#endif