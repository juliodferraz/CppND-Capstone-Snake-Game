#include "matrix.h"

Matrix::Matrix(const int& n_rows, const int& n_cols)
  : n_rows_(n_rows), 
    n_cols_(n_cols) {
    // data_(n_rows*n_cols, 0) {
  assert(sizeof(int) == 4);
  //tf_buffer_ = new MatrixBuffer(data_.data(), data_.size()*sizeof(int));
  //tf_tensor_ = Tensor(DT_INT32, TensorShape({n_rows_,n_cols_}), tf_buffer_);
  tf_tensor_ = Tensor(DT_INT32, TensorShape({n_rows_,n_cols_}));

  //Initialize tensor data with 0 values
  Reset();
}

Matrix& Matrix::Reset() {
  auto data = tf_tensor_.tensor<int, 2>();
  for(int row = 0; row < n_rows_; row++) {
      for(int col = 0; col < n_cols_; col++) {
        data(row, col) = 0;
      }
  }
  return *this;
}

// row and column are indexed from 0 to n_rows_/n_cols_ - 1.
int& Matrix::operator()(const int& row, const int& col) {
    assert(row >= 0 && row < n_rows_);
    assert(col >= 0 && col < n_cols_);
    //return data_[row*n_cols_ + col];
    //return tf_buffer_->base<int>()[row*n_cols_ + col];
    //return tf_tensor_.matrix<int>()(row+1,col+1);
    return tf_tensor_.tensor<int, 2>()(row, col);
}

// Returns const reference (for when matrix needs only to be read, and not written).
const int& Matrix::GetAt(const int& row, const int& col) const {
    assert(row >= 0 && row < n_rows_);
    assert(col >= 0 && col < n_cols_);
    //return data_[row*n_cols_ + col];
    //return tf_buffer_->base<int>()[row*n_cols_ + col];
    //return tf_tensor_.matrix<int>()(row+1,col+1);
    return tf_tensor_.tensor<int, 2>()(row, col);
}

// TODO: comment
//Matrix& Matrix::operator=(const Tensor& tensor) { 
    // Copy data from input tensor. Is this the most efficient? Can the data be moved instead?
    //assert(tf_tensor_.CopyFrom(tensor, TensorShape({n_rows_,n_cols_})) == true);
Matrix& Matrix::operator=(Tensor&& tensor) { 
    // Move data from input tensor
    tf_tensor_ = tensor;
    return *this;
}