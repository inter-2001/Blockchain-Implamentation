#pragma once
#include "CRYSALIS_DEFS.hpp"
#include "MATRIX.hpp"
#include "POLYN.hpp"
#include "cthash/sha3/shake128.hpp"
#include "cthash/sha3/shake256.hpp"
#include "plusaes.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <linux/limits.h>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <unistd.h>

typedef Matrix<POLYN> POLYN_MATRIX;

namespace KRYSALIS {
// Consts and typedef

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Randomiser Code
static std::random_device RD;
static std::mt19937 mt(RD());
static std::uniform_int_distribution<uint64_t> RandInt32(1, UINT32_MAX);
static std::uniform_int_distribution<uint8_t> RandInt8(1, UINT8_MAX);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONSTANT TIME COMPARE
template <typename DATATYPE> static bool CTC(DATATYPE ONE, DATATYPE TWO) {
  char eual = 0;
  if (ONE.length() == TWO.length()) {
    for (int i = 0; i < ONE.length(); i++) {
      eual |= ONE[i] ^ TWO[i];
    }
  } else {
    return (false);
  }
  return (eual == 0);
}

static int GET_BIT(int data, int i) {
  int bit = 0;
  bit = data & (1 << i); // creates a number that has 1 at the position that we
                         // want, we then and it with the data to get wether
                         // there is also a 1 there
  if (bit) {
    return (1);
  }
  return (0);
}

template <typename DATATYPE> static void FLIP_BIT(DATATYPE &data, int i) {
  data = data ^ (1 << i);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MODULAR DISTANCE CALCULATION
// Gives the magnitude of the number from clim/2

static BASEDATATYPE MODULAR_DIST(BASEDATATYPE Val) {
  if (clim - Val < Val) {
    return (clim - Val);
  }
  return (Val);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ENCODE/DECODE STRING TO/FROM POLYNNOMIAL
// CHANGED SINCE LAST WORKING
template <typename DATATYPE> static POLYN ENCODE_TO_POLLY(DATATYPE Data) {
  assert(sizeof(Data[0]) == sizeof(char));
  if (Data.length() < SEED_LEN) {
    std::string BUFF((SEED_LEN)-Data.length(), ' ');
    Data.insert(Data.end(), BUFF.begin(), BUFF.end());
  }

  if (Data.length() > SEED_LEN) {
    throw std::invalid_argument("ENCODE LENGTH " +
                                std::to_string(Data.length()) +
                                " OVER MAX OF: " + std::to_string(SEED_LEN));
  }
  POLYN RetVal;
  for (int i = 0; i < RetVal.EXPON_LEN; i++) {
    RetVal.SET(i, (clim / 2) * GET_BIT((int)Data[i / 8], i % 8));
  }
  return (RetVal);
}

template <typename DATATYPE> static DATATYPE DECODE_FROM_POLLY(POLYN Data) {
  DATATYPE RetVal;
  for (int i = 0; i < ELIM; i += 8) {
    char datum = 0;
    for (int j = 0; j < 8; j++) {
      if (MODULAR_DIST(Data.GET(i + j)) >
          clim / 4) { // this step essentially cuts out the noise around the
        // message, snapping it to the closest point on the lattice
        FLIP_BIT(datum, j);
      }
    }
    RetVal.insert(RetVal.end(), datum);
  }
  return (RetVal);
}

static POLYN ENCODE(int Datum) {
  if (sizeof(Datum) * 8 > ELIM) {
    throw std::invalid_argument("ENCODE LENGTH " +
                                std::to_string(sizeof(Datum)) +
                                " OVER MAX OF: " + std::to_string(ELIM / 8));
  }
  POLYN RetVal;
  for (int i = 0; i < ELIM; i++) {
    RetVal.ADDTERM(clim / 2 * GET_BIT(Datum, i), i);
  }
  RetVal.ADDTERM_FLUSH();
  return (RetVal);
}

template <typename DATATYPE> static DATATYPE DECODE(POLYN Data) {
  DATATYPE RetVal = 0;
  for (int i = 0; i < ELIM; i++) {
    if (MODULAR_DIST(Data.GET(i)) > clim / 4) {
      FLIP_BIT(RetVal, i);
    }
  }
  return (RetVal);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ KEY SEED GENERATOR

static std::string GEN_SEED() {
  std::string seed;
  for (size_t i = 0; i < SEED_LEN; ++i) {
    seed.push_back(RandInt8(mt));
  }
  return seed;
}

static std::vector<unsigned char> GEN_AES_KEY(std::string SEED) {
  cthash::shake256 HASHER;
  char KeySeed[33] = "";
  HASHER.update(SEED);
  auto RetHash = HASHER.final<8 * 33>();
  int i = 0;
  for (auto ELEM : RetHash) {
    KeySeed[i] = (char)ELEM;
  }
  return (plusaes::key_from_string(&KeySeed));
}

static std::string GEN_AES_VECTOR() {
  std::string SEED = GEN_SEED();
  cthash::shake256 HASHER;
  std::string KeySeed;
  HASHER.update(SEED);
  auto RetHash = HASHER.final<8 * 16>();
  for (auto ELEM : RetHash) {
    KeySeed.push_back((char)ELEM);
  }
  return (KeySeed);
}

// Centered Binomial Distrabution Mapping
template <typename DATATYPE>
static POLYN_MATRIX CBDMAP(const DATATYPE &SEED, int &N, int mod, int C,
                           int R) {
  assert(mod < 8);
  POLYN_MATRIX RetVal(C, R);
  if constexpr (DEBUG) {
    START_PROGRESS_BAR(ELIM * C * R, "GENERATE_CBD_MAP: ");
  }

  cthash::shake256 HASHER;
  HASHER.update(std::to_string(N));
  HASHER.update(SEED);
  auto RetHash = HASHER.final<sizeof(unsigned) * 8 * 1024>();

  for (int i = 0; i < C; i++) {
    for (int j = 0; j < R; j++) {
      for (int k = 0; k < ELIM; k++) {
        if constexpr (DEBUG) {
          PROGRESS_BAR();
        }

        unsigned A = 0;
        A |= (unsigned)RetHash[(i * R) + (j * ELIM) + (k * sizeof(unsigned))];
        for (int l = 1; l < sizeof(unsigned); l++) {
          A = A << 8;
          A |= (unsigned)
              RetHash[(i * R) + (j * ELIM) + (k * sizeof(unsigned)) + l];
        }
        unsigned MOD_SHIFT = (sizeof(unsigned) * 8 - mod);
        unsigned char a = (A >> (MOD_SHIFT));
        unsigned char b = ((A << MOD_SHIFT) >> MOD_SHIFT);
        char Sa = std::popcount(a);
        char Sb = std::popcount(b);
        S_BASEDATATYPE TERM;
        TERM = Sa - Sb;
        RetVal.GetActualVal(i, j)->SET(k, TERM);
      }
    }
  }
  N++;

  return (RetVal);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ POLLYNOMIAL MATRIX FROM SEED GENERATOR

static POLYN GENERATE_POLYN(const std::string &SEED, BASEDATATYPE MOD = clim) {
  BASEDATATYPE BITS_FOR_MOD = 0;
  if (MOD != clim) {
    BITS_FOR_MOD = std::bit_width(MOD);

  } else {
    BITS_FOR_MOD = CLIM_BITS;
  }

  if (!(MOD & (MOD - 1))) { // if mod is a power of 2
    BITS_FOR_MOD -= 1;
  }

  POLYN RetVal;

  if constexpr (DEBUG) {
    if (!BAR_STARTED) {
      START_PROGRESS_BAR(RetVal.EXPON_LEN, "GENERATE_POL: ");
    }
  }

  cthash::shake128 HASHER;
  HASHER.update(SEED);
  auto RetHash = HASHER.final<sizeof(unsigned) * 8 * ELIM * 16>();
  int m = 0;
  for (int k = 0; k < RetVal.EXPON_LEN; k++) {
    if constexpr (DEBUG) {
      PROGRESS_BAR();
    }
    unsigned datum = MOD;
    while (datum >= MOD) {

      datum = 0;
      datum |= (unsigned)RetHash[m];
      for (int l = 1; l < sizeof(unsigned); l++) {
        datum = datum << 8;
        datum |= (unsigned)RetHash[m + l];
      }
      datum = datum >> (sizeof(unsigned) * 8 - BITS_FOR_MOD);
      m += sizeof(unsigned) + (BITS_FOR_MOD / 8);
    }
    RetVal.ADDTERM(static_cast<BASEDATATYPE>(datum), k);
  }
  RetVal.ADDTERM_FLUSH();
  return (RetVal);
}

static POLYN_MATRIX GENERATE_POLYN_MATRIX(const std::string &SEED, unsigned C,
                                          unsigned R, BASEDATATYPE MOD = clim) {

  POLYN_MATRIX RetVal(C, R);
  BASEDATATYPE BITS_FOR_MOD = 0;
  if (MOD != clim) {
    BITS_FOR_MOD = std::bit_width(MOD);

  } else {
    BITS_FOR_MOD = CLIM_BITS;
  }

  if (!(MOD & (MOD - 1))) { // if mod is a power of 2
    BITS_FOR_MOD -= 1;
  }

  if constexpr (DEBUG) {
    START_PROGRESS_BAR(RetVal.GET_ITER(0).EXPON_LEN * C * R,
                       "GENERATE_POL_MATRIX: ");
  }

  cthash::shake128 HASHER;
  HASHER.update(SEED);
  auto RetHash = HASHER.final<sizeof(unsigned) * 128 * ELIM>();
  if (C * R <= 4) {
    auto RetHash = HASHER.final<sizeof(unsigned) * 128 * ELIM>();
  }
  for (int i = 0; i < C; i++) {
    for (int j = 0; j < R; j++) {
      int m = 0;
      for (int k = 0; k < RetVal.Data[i][j].EXPON_LEN; k++) {
        if constexpr (DEBUG) {
          PROGRESS_BAR();
        }
        unsigned datum = MOD;
        while (datum >= MOD) {
          datum = 0;
          datum |= (unsigned)RetHash[m];
          for (int l = 1; l < (sizeof(unsigned) * 8 - BITS_FOR_MOD) / 8; l++) {
            datum = datum << 8;
            datum |= (unsigned)RetHash[m + l];
          }
          datum = datum >> (sizeof(unsigned) * 8 - BITS_FOR_MOD) % 8;
          m += sizeof(unsigned) + (BITS_FOR_MOD / 8);
        }
        RetVal.Data[i][j].ADDTERM(static_cast<BASEDATATYPE>(datum), k);
      }
      RetVal.Data[i][j].ADDTERM_FLUSH();
    }
  }

  return (RetVal);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RANDOM FUNCTIONS

static unsigned char RandN() { return (RandInt8(mt) / 8) * 8; }

} // namespace KRYSALIS
