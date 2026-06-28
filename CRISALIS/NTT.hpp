

#include "CRYSALIS_DEFS.hpp"
#include <sys/types.h>
#include <vector>

#define BYTE_ARRAY std::vector<u_char>
#define EXPONENT 256
#define COEFICNT 3329

class POLYN {
private:
  int DATA[EXPONENT];

public:
  POLYN() {
    for (int i = 0; i < EXPONENT; i++) {
      DATA[i] = 0;
    }
  }
  int GET(int i) const { return DATA[i]; }
  void SET(int i, int VAL) { DATA[i] = VAL; }
};

static BYTE_ARRAY BYTES_TO_BITS(BYTE_ARRAY B) {
  BYTE_ARRAY Ret;
  for (u_char ITEM : B) {
    for (int i = 0; i < 8; i++) {
      Ret.emplace_back(GET_BIT(ITEM, i));
    }
  }
  return (Ret);
};

const int n = EXPONENT;
const int q = COEFICNT;

static POLYN PARSE(BYTE_ARRAY B) {
  POLYN A;
  int i = 0;
  int j = 0;
  while (j < EXPONENT) {
    int d1 = B[i] + 256 & (B[i + 1] % 16);
    int d2 = (B[i + 1] / 16) + 16 & B[i + 2];
    if (d1 < q) {
      A.SET(j, d1);
      j++;
    }
    if (d2 < q && j < EXPONENT) {
      A.SET(j, d2);
      j++;

      i += 3;
    }
  }
  return (A);
}

static POLYN CBD(int Nu, BYTE_ARRAY B) {
  BYTE_ARRAY Bits = BYTES_TO_BITS(B);
  POLYN Ret;
  for (int i = 0; i < EXPONENT; i++) {
    int a = 0;
    int b = 0;
    for (int j = 0; j < Nu; j++) {
      a += Bits[(2 * i * Nu + j)];
      b += Bits[(2 * i * Nu + Nu + j)];
      Ret.SET(i, a - b);
    }
  }
  return (Ret);
}

static POLYN Decode(BYTE_ARRAY B) {
  BYTE_ARRAY Bits = BYTES_TO_BITS(B);
  POLYN Ret;
  for (int i = 0; i < EXPONENT; i++) {
    for
      int(j = 0; j < B.size(); j++) { Ret[i] = Bits[i + j] }
  }
}
