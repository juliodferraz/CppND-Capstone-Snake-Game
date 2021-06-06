#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <memory>

// Signed int matrix (4 bytes per element, int32)
class Matrix {
 public:
  // Vector is initialized filled with zeros
  Matrix(const int& n_rows, const int& n_cols);

  // row and column are indexed from 0 to n_rows_/n_cols_ - 1.
  int& operator()(const int& row, const int& col);

  // Returns const reference (for when matrix needs only to be read, and not written).
  const int& GetAt(const int& row, const int& col) const;

  // Clears and resets all matrix elements to 0 (default value)
  Matrix& Reset(); //{ data_.assign(n_rows_*n_cols_, 0); }

  // print operator
  friend std::ostream& operator<<(std::ostream& out, const Matrix& matrix) {
    for(int row = 0; row < matrix.n_rows_; row++) {
        out << "Row " << row << ": [";
        for(int col = 0; col < matrix.n_cols_; col++) {
          out << " " << data[row][col];
        }
        out << "]" << std::endl;
    }
    return out;
  }

 private:
  int n_rows_;
  int n_cols_;
  std::unique_ptr<std::unique_ptr<int>> data;
};

#endif