#pragma once
#include "CRYSALIS_DEFS.hpp"
#include "SERIALIZER.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

template <typename BASE_TYPE> class POLYNOMIAL {
public:
  // DATA[i] is the coefficient for x^i, for i in [0, ARRAY_LEN)
  BASE_TYPE *DATA = nullptr;
  BASE_TYPE DATUM_LEN = CLIM_BITS;
  BASE_TYPE EXPON_LEN = ELIM;
  RES_DATATYPE *ADDTERM_BUFFER = new RES_DATATYPE[EXPON_LEN];
  BASE_TYPE ARRAY_LEN = 0;
  int MODE = 0;
  static const BASE_TYPE SECTR_LEN = sizeof(BASE_TYPE) << 3;
  BASE_TYPE DATUM_LIM() const { return (1 << (DATUM_LEN)); }
  int SIZE() { return ((sizeof(BASE_TYPE) * ARRAY_LEN) + 2); }

  ~POLYNOMIAL() {
    delete[] (DATA);
    delete[] (ADDTERM_BUFFER);
  }

  // GUF area

  void INITILIZE() { SET_ARR_LEN(); }

  void SET_ARR_LEN() {
    ARRAY_LEN = ((EXPON_LEN * DATUM_LEN) / SECTR_LEN);
    if ((EXPON_LEN * DATUM_LEN) % SECTR_LEN) {
      ARRAY_LEN += 1;
    }
    if (DATA != nullptr) {
      delete[] DATA;
    }
    DATA = new BASE_TYPE[ARRAY_LEN];
    for (int i = 0; i < ARRAY_LEN; ++i) {
      DATA[i] = 0;
    }
    RESET_TERM_BUFFER();
  }

  void RESET_TERM_BUFFER() {
    for (int i = 0; i < EXPON_LEN; i++) {
      ADDTERM_BUFFER[i] = 0;
    }
  }

  /* ADD the term Cx^E to the polynomial add buffer*/
  void ADDTERM(BASE_TYPE C, BASE_TYPE E) {
    if (E >= 0 && E < EXPON_LEN && C != 0) {
      ADDTERM_BUFFER[E] += C;
    }
  }
  BASE_TYPE GET_TERM(BASE_TYPE E) { return (ADDTERM_BUFFER[E]); }

  void ADDTERM_FLUSH() {
    for (int i = 0; i < EXPON_LEN; i++) {

      SET(i, (BASEDATATYPE)INDIV_LIMIT(ADDTERM_BUFFER[i] + GET(i)));
    }
    for (int i = 0; i < EXPON_LEN; i++) {
      ADDTERM_BUFFER[i] = 0;
    }
  }

  void MOVE_TO_BUFFER() {
    for (int i = 0; i < EXPON_LEN; i++) {
      ADDTERM_BUFFER[i] = GET(i);
    }
  }

  void ZERO() {
    for (int i = 0; i < ARRAY_LEN; ++i) {
      DATA[i] = 0;
    }
  }
  int length() { return static_cast<int>(ARRAY_LEN); }

  BASE_TYPE &operator[](int index) {
    if (index < 0 || index > ARRAY_LEN) {
      throw std::invalid_argument("ARRAY INDEX OUT OF BOUNDS");
    } else {
      return (DATA[index]);
    }
  }

  // CONSTRUCTORS

  POLYNOMIAL() { // zero-initialize
    INITILIZE();
  }

  POLYNOMIAL(BASE_TYPE Length) { // zero-initialize
    EXPON_LEN = Length;
    INITILIZE();
  }

  POLYNOMIAL(BASE_TYPE Length, BASE_TYPE DatumLength) { // zero-initialize
    EXPON_LEN = Length;
    DATUM_LEN = DatumLength;
    INITILIZE();
  }

  POLYNOMIAL(const POLYNOMIAL &other)
      : DATA(nullptr), DATUM_LEN(other.DATUM_LEN), EXPON_LEN(other.EXPON_LEN),
        ARRAY_LEN(0) {
    SET_ARR_LEN();
    for (int i = 0; i < ARRAY_LEN; ++i) {
      DATA[i] = other.DATA[i];
    }
  }

  POLYNOMIAL(const BASE_TYPE arr[], int len) { // build from raw array
    DATUM_LEN = sizeof(BASE_TYPE) * 8;
    EXPON_LEN = len;
    INITILIZE();
  }

  // STRING SERIALIZATION

  POLYNOMIAL(std::string ENCODED_POLYNOMIAL) {
    EXPON_LEN = (unsigned char)(ENCODED_POLYNOMIAL[0] * UINT8_MAX);
    EXPON_LEN += (unsigned char)(ENCODED_POLYNOMIAL[1]);
    DATUM_LEN = (unsigned char)ENCODED_POLYNOMIAL[2];
    unsigned FOLLOWER = 3;
    unsigned CHARS = sizeof(BASE_TYPE);
    INITILIZE();
    for (int i = 0; i < ARRAY_LEN; i++) {
      DATA[i] =
          DESERIALIZE<BASE_TYPE>(ENCODED_POLYNOMIAL.substr(FOLLOWER, CHARS));
      FOLLOWER += CHARS;
    }
  }

  std::string DATA_SERIALIZE() {
    std::string RetVal = "";
    RetVal += (unsigned char)(EXPON_LEN / UINT8_MAX);
    RetVal += (unsigned char)(EXPON_LEN % UINT8_MAX);
    RetVal += (unsigned char)DATUM_LEN;
    for (int i = 0; i < ARRAY_LEN; i++) {
      RetVal += SERIALIZE(DATA[i]);
    }
    return (RetVal);
  }

  // GET AND SET FUNCTIONS

  S_BASEDATATYPE GET(unsigned POS) const {
    POS = POS * DATUM_LEN;
    BASE_TYPE negative = -1;
    negative = negative << CLIM_BITS;

    if (!((POS + DATUM_LEN) - 1 < SECTR_LEN * ARRAY_LEN)) {
      std::cout << "GET_ERROR" << std::endl;
      std::cout << "   ARRAY_LEN: " << std::to_string(ARRAY_LEN) << std::endl;
      std::cout << "   EXPON_LEN: " << std::to_string(EXPON_LEN) << std::endl;
      std::cout << "   DATUM_LEN: " << std::to_string(DATUM_LEN) << std::endl;
      std::cout << "   EXPON_IND: " << std::to_string(POS) << std::endl;
      assert(false);
    }

    BASEDATATYPE RetVal = 0;

    for (int i = 0; i < DATUM_LEN; i++) {
      BASE_TYPE ARRAY_POS = (POS + i) / SECTR_LEN;
      BASE_TYPE DATUM_POS = (POS + i) % SECTR_LEN;
      RetVal |= GET_BIT(DATA[ARRAY_POS], DATUM_POS) << i;
    }

    if (RetVal > clim) {
      RetVal |= negative;
    }
    return ((BASE_TYPE)RetVal);
  }

  void SET(unsigned POS, S_BASEDATATYPE val) {
    POS *= DATUM_LEN;

    BASE_TYPE VAL = INDIV_LIMIT(val);
    for (int i = 0; i < DATUM_LEN; i++) {
      BASE_TYPE ARRAY_POS = (POS + i) / SECTR_LEN;
      BASE_TYPE DATUM_POS = (POS + i) % SECTR_LEN;
      DATA[ARRAY_POS] |= 1 << (DATUM_POS);
      DATA[ARRAY_POS] ^= !GET_BIT(VAL, i) << (DATUM_POS);
    }
  }

  // COMPRESS AND DECOMPRESS FUNCTIONS

  POLYNOMIAL COMPRESS(BASE_TYPE NEW_WIDTH) {
    POLYNOMIAL RetVal(EXPON_LEN, NEW_WIDTH);
    for (int i = 0; i < EXPON_LEN; i++) {
      unsigned SCALED = (GET(i) << NEW_WIDTH) + clim / 2;
      RetVal.SET(i, SCALED / 2);
    }
    return (RetVal);
  }

  POLYNOMIAL DECOMPRESS(BASE_TYPE OLD_WIDTH) {
    POLYNOMIAL RetVal(EXPON_LEN);
    for (int i = 0; i < EXPON_LEN; i++) {
      unsigned SCALED = (GET(i) * clim) + (1 << (OLD_WIDTH - 1));

      RetVal.SET(i, SCALED >> OLD_WIDTH);
    }
    return (RetVal);
  }

  // PRINT FUNCTIONS

  friend std::ostream &operator<<(std::ostream &os, const POLYNOMIAL &Polly) {
    return os << Polly.PRINT();
  }
  operator std::string() const { return PRINT(); }

  std::string PRINT() const {
    std::string RetVal = "";
    for (int i = 0; i < EXPON_LEN; ++i) {
      if (GET(i) != 0) {
        if (GET(i) > 0 && !RetVal.empty()) {
          RetVal += "+";
        }
        RetVal += (std::to_string(GET(i)) + "x^" + std::to_string(i) + " ");
      }
    }
    return RetVal;
  }

  void PRINT_RAW_ARAY() {
    std::cout << "\n" << PRINT() << std::endl;
    for (int i = 0; i < ARRAY_LEN; i++) {
      std::cout << PRINT_BITS(DATA[i]) << " : ";
      if (!((i + 1) % 4)) {
        std::cout << std::endl;
      }
    }
  }

  void PRINT_RAW_EXPS() {
    std::cout << "\n" << PRINT() << std::endl;
    for (int i = 0; i < EXPON_LEN; i++) {
      std::cout << PRINT_BITS(GET(i)) << " : ";
      if (!((i + 1) % 4)) {
        std::cout << std::endl;
      }
    }
  }

  // EQUALS OPERATORS

  void operator=(const POLYNOMIAL &other) {
    if (this == &other) {
      return;
    }

    EXPON_LEN = other.EXPON_LEN;
    DATUM_LEN = other.DATUM_LEN;

    SET_ARR_LEN();
    for (int i = 0; i < ARRAY_LEN; ++i) {
      DATA[i] = other.DATA[i];
    }
  }

  void operator=(BASE_TYPE other) {
    for (int i = 0; i < EXPON_LEN; ++i) {
      SET(i, 0);
    }
    ADDTERM(other, 0);
  }

  bool operator==(const POLYNOMIAL &other) {
    if (EXPON_LEN != other.EXPON_LEN) {
      return (false);
    }
    for (int i = 0; i < EXPON_LEN; i++) {
      if (GET(i) != other.GET(i)) {
        return (false);
      }
    }
    return (true);
  }

  bool operator!=(const POLYNOMIAL &other) { return (!(*this == other)); }

  // Mathamatical Operators Overload

  // PLUS OPERATOR OVERLOAD
  POLYNOMIAL
  operator+(const POLYNOMIAL & other) const { // coefficient-wise add
    POLYNOMIAL Ret(EXPON_LEN, DATUM_LEN);
    for (int i = 0; i < EXPON_LEN; i++) {
      long long int Res = GET(i) + other.GET(i);
      Ret.SET(i, INDIV_LIMIT(Res));
    }
    return Ret;
  }

  void operator+=(POLYNOMIAL other) { // coefficient-wise add
    for (int i = 0; i < EXPON_LEN; i++) {
      long long int Res = GET(i) + other.GET(i);
      SET(i, INDIV_LIMIT(Res));
    }
  }

  // SUBTRACT OPERATOR OVERLOAD
  POLYNOMIAL
  operator-(const POLYNOMIAL & other) const { // coefficient-wise subtract
    POLYNOMIAL Ret(EXPON_LEN, DATUM_LEN);
    for (int i = 0; i < EXPON_LEN; i++) {
      long long int Res = GET(i) - other.GET(i);
      Ret.SET(i, INDIV_LIMIT(Res));
    }
    return Ret;
  }
  void operator-=(POLYNOMIAL other) {
    for (int i = 0; i < EXPON_LEN; i++) {
      long long int Res = GET(i) - other.GET(i);
      SET(i, INDIV_LIMIT(Res));
    }
  }

  POLYNOMIAL DIFERANCE(POLYNOMIAL &other) {
    POLYNOMIAL Ret(EXPON_LEN, DATUM_LEN);
    for (int i = 0; i < EXPON_LEN; i++) {
      long long int Res = GET(i) - other.GET(i);
      Ret.SET(i, std::abs(Res));
    }
    return (Ret);
  }

  // MULTIPLY OPERATOR OVERLOAD
  // negacyclic convolution
  // returns a polynomoal of type BASEDATATYPE
  POLYNOMIAL operator*(POLYNOMIAL other) const {
    assert(EXPON_LEN == other.EXPON_LEN);
    POLYNOMIAL RetVal(EXPON_LEN, DATUM_LEN);
    other.MOVE_TO_BUFFER();
    for (int i = 0; i < EXPON_LEN; i++) {
      RES_DATATYPE THIS_I = (RES_DATATYPE)GET(i);
      for (int j = 0; j < EXPON_LEN; j++) {
        int k = i + j;
        RES_DATATYPE COE = THIS_I * (RES_DATATYPE)other.GET_TERM(j);
        int SIGN = 1;
        if (k >= EXPON_LEN) {
          SIGN = (((k / EXPON_LEN) % 2) * -2) + 1;
          k = k % EXPON_LEN;
        }
        RES_DATATYPE RET = INDIV_LIMIT(COE * SIGN);
        RetVal.ADDTERM((BASEDATATYPE)RET, k);
      }
    }
    RetVal.ADDTERM_FLUSH();
    return (RetVal);
  }

  // MODULO OPERATOR OVERLOAD
  POLYNOMIAL operator%(const BASE_TYPE &Limit) {
    POLYNOMIAL RetVal = *this;
    RetVal.LIMIT();
    return RetVal;
  }

  // LIMIT FUNCTIONS

  template <typename VALTYPE>
  BASE_TYPE INDIV_LIMIT(VALTYPE val) const { // reduce coefficients modulo clim
    RES_DATATYPE COE = (RES_DATATYPE)val % (RES_DATATYPE)clim;
    if (COE < 0) {
      COE += (RES_DATATYPE)clim;
    }
    return (COE);
  }

  void LIMIT() { // reduce coefficients modulo clim
    for (int i = 0; i < EXPON_LEN; ++i) {
      SET(i, INDIV_LIMIT((long long int)GET(i)));
    }
  }
};

typedef POLYNOMIAL<BASEDATATYPE> POLYN;
