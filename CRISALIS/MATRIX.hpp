#pragma once
#include "SERIALIZER.hpp"
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>

template <typename DATATYPE> class Matrix {
public:
  unsigned char C = 1;
  unsigned char R = 1;
  DATATYPE **Data = nullptr;

  void InitiliseData() { // initilise all the pointers needed then set all the
    if (C < 1) {
      C = 1;
    }
    if (R < 1) {
      R = 1;
    }
    // pointed to datapoints to zero
    Data = new DATATYPE *[C];
    for (int i = 0; i < C; i++) {
      Data[i] = new DATATYPE[R];
    }
  }

  void DeleteData() { // go through each coll and delete the pointer to the
    for (int i = 0; i < C; i++) {
      delete[] Data[i];
    }
    delete[] Data;
  }

  void CLEAR() {
    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Data[i][j] = 0;
      }
    }
  }

  static DATATYPE DotProduct(const Matrix &M1, const Matrix &M2) {
    DATATYPE Res;

    if ((M1.C * M1.R == (M1.C + M1.R) - 1) &&
        (M2.C * M2.R == (M2.C + M2.R) - 1)) {

      if (M1.C * M1.R == M2.C * M2.R) {
        for (int i = 0; i < M1.C * M1.R; i++) {
          Res += M1.CONST_GET_ITER(i) * M2.CONST_GET_ITER(i);
        }
        return (Res);
      } else {
        throw std::invalid_argument("Vector length Mismatch");
      }
    } else {
      throw std::invalid_argument("Passed non-Vector");
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Constructors
  //

  Matrix(int c, int r) : C(c), R(r) { // constructor
    InitiliseData();
  };

  Matrix() : C(1), R(1) { // constructor
    InitiliseData();
  };

  template <typename O_DATA_TYPE>
  Matrix(const Matrix<O_DATA_TYPE> &Origin) { // copy constructor
    C = Origin.C;
    R = Origin.R;
    InitiliseData();
    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Data[i][j] = (DATATYPE)Origin.Data[i][j];
      }
    }
  }

  Matrix(const Matrix &other) : C(other.C), R(other.R) {
    InitiliseData();
    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Data[i][j] = other.Data[i][j];
      }
    }
  }

  template <typename O_DATA_TYPE>
  void operator=(const Matrix<O_DATA_TYPE>
                     &Origin) { // defining what to do if someone tries to
                                // say class = otherInstanceOfSameClass

    if ((void *)this ==
        (void *)&Origin) { // if the origin is the same as this then
      return;
    }

    DeleteData(); // allways delete the data before resetting C and R

    C = Origin.C;
    R = Origin.R;

    InitiliseData();

    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Data[i][j] = (DATATYPE)Origin.Data[i][j];
      }
    }

    return;
  }

  void
  operator=(const Matrix &Origin) { // defining what to do if someone tries to
                                    // say class = otherInstanceOfSameClass

    if (this == &Origin) { // if the origin is the same as this then
      return;
    }

    DeleteData(); // allways delete the data before resetting C and R

    C = Origin.C;
    R = Origin.R;

    InitiliseData();

    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Data[i][j] = Origin.Data[i][j];
      }
    }

    return;
  }

  // SERIALIZE AND DESERIALIZE MATRIX
  Matrix(std::string SERIAL_MATRIX) {
    R = SERIAL_MATRIX[0];
    C = SERIAL_MATRIX[1];
    InitiliseData();
    unsigned PREV = 2;
    for (int i = 0; i < R * C; i++) {
      unsigned EXTENT = (unsigned char)SERIAL_MATRIX[PREV] * UINT8_MAX;
      EXTENT += (unsigned char)SERIAL_MATRIX[PREV + 1];
      std::string TEMP = SERIAL_MATRIX.substr(PREV + 2, EXTENT);
      GET_ITER(i) = DATATYPE(TEMP);
      PREV += EXTENT + 2;
    }
  }

  std::string DATA_SERIALIZE() {
    std::string RetVal = "";
    RetVal += R;
    RetVal += C;
    for (int i = 0; i < R * C; i++) {
      std::string TEMP = "";
      if constexpr (std::is_trivially_copyable_v<DATATYPE>) {
        TEMP += SERIALIZE(GET_ITER(i));
      } else {
        TEMP += GET_ITER(i).DATA_SERIALIZE();
      }

      unsigned TEMP_LEN = TEMP.length();
      RetVal += (unsigned char)(TEMP_LEN / UINT8_MAX);
      RetVal += (unsigned char)(TEMP_LEN % UINT8_MAX);
      RetVal += TEMP;
    }
    return (RetVal);
  }

  bool operator==(Matrix &OTHER) {
    if (C != OTHER.C) {
      return (false);
    }
    if (R != OTHER.R) {
      return (false);
    }
    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        if (GetVal(i, j) != OTHER.GetVal(i, j)) {
          return (false);
        }
      }
    }
    return (true);
  }

  bool operator!=(Matrix &OTHER) { return (!(*this == OTHER)); }

  DATATYPE &GET_ITER(int i) {
    int c = i % C;
    int r = i / C;
    return (Data[c][r]);
  }

  DATATYPE CONST_GET_ITER(int i) const {
    int c = i % C;
    int r = i / C;
    return (GetVal(c, r));
  }

  Matrix(std::initializer_list<std::initializer_list<DATATYPE>> INITARR) {
    C = std::size(INITARR);
    R = std::size(INITARR[0]);
    InitiliseData();
    for (int i = 0; i < C; i++) {
      for (int j = 0; i < R; i++) {
        Data[i][j] = INITARR[i][j];
      }
    }
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Data Reinitilization
  void Reinitilize(int Cols, int Rows) {
    DeleteData();
    C = Cols;
    R = Rows;
    InitiliseData();
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Functions to apply a given
  // function to each datum in the matrix can also be used to gen custom
  // matrixes

  void ApplyFunction(DATATYPE (*Func)(int c, int r, DATATYPE DATUM)) {
    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Data[i][j] = Func(i, j, Data[i][j]);
      }
    }
  }

  void ApplyFunction(DATATYPE (*Func)()) {
    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Data[i][j] = Func();
      }
    }
  }

  Matrix COMPRESS(int NEW_BIT_WIDTH) {
    Matrix RetVal = *this;
    for (int i = 0; i < R * C; i++) {
      RetVal.GET_ITER(i).COMPRESS(NEW_BIT_WIDTH);
    }
    return (RetVal);
  }

  Matrix DECOMPRESS(int OLD_BIT_WIDTH) {
    Matrix RetVal = *this;
    for (int i = 0; i < R * C; i++) {
      RetVal.GET_ITER(i).DECOMPRESS(OLD_BIT_WIDTH);
    }
    return (RetVal);
  }

  ~Matrix() { DeleteData(); };

  // ease of life Functions

  static std::string AS_STRING(const DATATYPE &value) {
    std::ostringstream TEMP;
    TEMP << value;
    return (TEMP.str());
  }

  void PrintMat() const {
    int ColWidth[C];
    int DatumWidth = 0;
    for (int j = 0; j < C; j++) {
      ColWidth[j] = 0;
    }
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < C; j++) {
        DatumWidth = static_cast<int>(AS_STRING(Data[j][i]).length());
        if (DatumWidth > ColWidth[j]) {
          ColWidth[j] = DatumWidth;
        }
      }
    }
    for (int j = 0; j < C; j++) {
      ColWidth[j] += 1;
    }

    std::cout << "\nC: " << std::to_string(C) << " R: " << std::to_string(R);
    for (int i = 0; i < R; i++) {
      std::cout << "\n";
      for (int j = 0; j < C; j++) {
        std::cout << Data[j][i];
        const int cellLen = static_cast<int>(AS_STRING(Data[j][i]).length());
        for (int k = 0; k < (ColWidth[j] - cellLen); k++) {
          std::cout << (" ");
        }
      }
    }
    std::cout << "\n";
    return;
  }

  int GetC() const { return (C); }

  int GetR() const { return (R); }

  // operatior settings for matrices

  template <typename O_DATATYPE>
  Matrix<DATATYPE> operator+(const Matrix<O_DATATYPE> &Origin) const {

    int NumColls = 0;
    int NumRows = 0;

    // output matrix C is the larger of origin or C
    if (Origin.C >= C) {
      NumColls = Origin.C;
    } else {
      NumColls = C;
    }

    // Output matrix R is the larger of origin or R
    if (Origin.R >= R) {
      NumRows = Origin.R;
    } else {
      NumRows = R;
    }

    Matrix Result(NumColls, NumRows);

    for (int i = 0; i < Origin.C; i++) {
      for (int j = 0; j < Origin.R; j++) {
        Result.Data[i][j] += Origin.Data[i][j];
      }
    }

    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Result.Data[i][j] += Data[i][j];
      }
    }
    return (Result);
  }

  template <typename O_DATATYPE>
  Matrix<DATATYPE> operator-(const Matrix<O_DATATYPE> &Origin) const {

    int NumColls = 0;
    int NumRows = 0;

    // output matrix C is the larger of origin or C
    if (Origin.C >= C) {
      NumColls = Origin.C;
    } else {
      NumColls = C;
    }

    // Output matrix R is the larger of origin or R
    if (Origin.R >= R) {
      NumRows = Origin.R;
    } else {
      NumRows = R;
    }

    Matrix Result(NumColls, NumRows);

    for (int i = 0; i < Origin.C; i++) {
      for (int j = 0; j < Origin.R; j++) {
        Result.Data[i][j] -= Origin.Data[i][j];
      }
    }

    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Result.Data[i][j] -= Data[i][j];
      }
    }

    return Result;
  }

  Matrix operator+(const int &Val) {
    Matrix Result(C, R);
    for (int i = 0; i < C; i++) {
      for (int j = 0; j < R; j++) {
        Result.Data[i][j] += Data[i][j] + Val;
      }
    }
    return (Result);
  }

  Matrix DIFERANCE(Matrix other) {
    Matrix RetVal(C, R);
    for (int i = 0; i < R * C; i++) {
      RetVal.GET_ITER(i) = GET_ITER(i).DIFERANCE(other.GET_ITER(i));
    }
    return (RetVal);
  }

  Matrix<DATATYPE> operator*(Matrix Origin) const {
    Matrix Result;

    if (C == Origin.R) {
      Result.Reinitilize(Origin.C, R);
      for (int i = 0; i < Result.C; i++) {
        for (int j = 0; j < Result.R; j++) {
          Result.Data[i][j] = DotProduct(GetRow(j), Origin.GetCol(i));
        }
      }
    } else {
      std::cout
          << "Matrix Multiplication Error:\nMatrix Width/Height Misatch\n";
      PrintMat();
      Origin.PrintMat();
      return (Result);
    }

    return (Result);
  }

  Matrix operator%(const int &Modulo) const {

    Matrix Result(C, R);
    for (int i = 0; i < Result.C; i++) {
      for (int j = 0; j < Result.R; j++) {
        Result.Data[i][j] = Data[i][j] % Modulo;
      }
    }
    return (Result);
  }

  // functions to set and get indavidual points within the matrix

  void SetVal(int c, int r, DATATYPE Val) {

    if (c >= C) {
      std::cout << "SetPoint Error: \nc val out of range\n";
      return;
    }

    else if (r >= R) {
      std::cout << "SetPoint Error: \nr val out of range\n";
      return;
    }

    else {
      Data[c][r] = Val;
      return;
    }
  }

  DATATYPE GetVal(int c, int r) const {

    if (c >= C) {
      throw std::invalid_argument("GetPoint Error: \nc val out of range\n");
    }

    else if (r >= R) {
      throw std::invalid_argument("GetPoint Error: \nr val out of range\n");
    }

    else {
      return (Data[c][r]);
    }
  }

  DATATYPE *GetActualVal(int c, int r) {

    if (c >= C) {
      throw std::invalid_argument("GetPoint Error: \nc val out of range\n");
    }

    else if (r >= R) {
      throw std::invalid_argument("GetPoint Error: \nr val out of range\n");
    }

    else {
      return (&Data[c][r]);
    }
  }

  // -----------------------------------------Set Row
  void SetRow(const Matrix &NewRow, int row) {
    for (int i = 0; i < C; i++) {
      Data[i][row] = NewRow.GET_ITER(i);
    }
    return;
  }

  // -----------------------------------------Set Row From Array
  void SetRowFromArray(DATATYPE *NewRow, int row) {
    try {
      for (int i = 0; i < C; i++) {
        Data[i][row] = NewRow[i];
      }
    } catch (...) {
      std::cout << "\nRowFromArray Error:\narray longer than matrix\n";
      return;
    }
    return;
  }

  // -----------------------------------------Get Row
  Matrix GetRow(int row) const {
    Matrix Result(C, 1);
    for (int i = 0; i < C; i++) {
      Result.Data[i][0] = Data[i][row];
    }
    return (Result);
  }

  // -----------------------------------------Set Col
  void SetCol(const Matrix &NewCol, int Col) {

    if (NewCol.R != R) {
      NewCol.PrintMat();
      PrintMat();
      std::cout << "\nR: " << R << " NEWCOL.R: " << NewCol.R << std::endl;
      std::cout << "SetColError:\nWrong number of Rows in new Col\n";
      return;
    } else if (NewCol.C > 1) {
      std::cout << "SetColError:\nmore than one Col in submitted matrix";
      return;
    } else {
      for (int i = 0; i < NewCol.R; i++) {
        Data[Col][i] = NewCol.Data[0][i];
      }
    }

    return;
  }

  // -----------------------------------------Get Col
  Matrix GetCol(int Col) const {
    Matrix Result(1, R);

    for (int i = 0; i < R; i++) {
      Result.Data[0][i] = Data[Col][i];
    }
    return (Result);
  }

  // -----------------------------------------Scalar Multiply

  Matrix ScalarMultiply(DATATYPE Multiplyer) const {
    Matrix Result(C, R);
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < C; j++) {
        Result.Data[i][j] = Data[j][i] * Multiplyer;
      }
    }
    return (Result);
  }

  // -----------------------------------------Scalar Add

  Matrix ScalarAddition(DATATYPE Adder) const {
    Matrix Result(C, R);
    for (int i = 0; i < R; i++) {
      for (int j = 0; j < C; j++) {
        Result.Data[i][j] = Data[j][i] + Adder;
      }
    }
    return (Result);
  }

  // -----------------------------------------Transpose

  Matrix Transpose() const {
    Matrix Result(R, C);
    for (int i = 0; i < C; i++) {
      Result.SetRow(GetCol(i), i);
    }
    return (Result);
  }
  // -----------------------------------------Append Matrix

  Matrix AppendMatrix(Matrix M1) const {

    int ResR = 0;
    if (M1.R > R) {
      ResR = M1.R;
    } else {
      ResR = R;
    }

    Matrix Result(C + M1.C, ResR);

    for (int i = 0; i < C; i++) {
      Result.SetCol(GetCol(i), i);
    }
    for (int i = 0; i < M1.C; i++) {
      Result.SetCol(M1.GetCol(i), i + C);
    }

    return (Result);
  }
};

// Class Independant Functions
