#include "matrix.h"

Matrix::Matrix(const int& n_rows, const int& n_cols) : n_rows_(n_rows), n_cols_(n_cols) {
  data = std::make_unique(n_rows_);
  for(int row = 0; row < n_rows_; row++) {
    data[row] = std::make_unique(n_cols_);
  }

  //Initialize matrix data with values of 0
  Reset();
}

Matrix& Matrix::Reset() {
  for(int row = 0; row < n_rows_; row++) {
      for(int col = 0; col < n_cols_; col++) {
        data[row][col] = 0;
      }
  }
  return *this;
}

// row and column are indexed from 0 to n_rows_/n_cols_ - 1.
int& Matrix::operator()(const int& row, const int& col) {
    assert(row >= 0 && row < n_rows_);
    assert(col >= 0 && col < n_cols_);
    return data[row][col];
}

// Returns const reference (for when matrix needs only to be read, and not written).
const int& Matrix::GetAt(const int& row, const int& col) const {
    assert(row >= 0 && row < n_rows_);
    assert(col >= 0 && col < n_cols_);
    return data[row][col];
}