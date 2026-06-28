#pragma once
#include "CRISALIS.hpp"
#include "CRYSALIS_DEFS.hpp"
#include "MATRIX.hpp"
#include "POLYN.hpp"
#include <linux/limits.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

using namespace KRYSALIS;

static unsigned short BASE_DIMENTIONS = 2;

// GENERATING BASE
static POLYN_MATRIX base =
    GENERATE_POLYN_MATRIX("abcdefghijklmnop", BASE_DIMENTIONS, BASE_DIMENTIONS);

struct Private_Key {
  std::string Key;
  std::string Error;
};

struct KeyPair {
  std::string PublicKey;
  Private_Key PrivateKey;
};

static std::string GeneratePrivateKey(std::string Passcode = "") {
  int SKEY_N = 0;
  POLYN_MATRIX SKEY = CBDMAP(Passcode, SKEY_N, SMALL_INT, 1, BASE_DIMENTIONS);
  return (SKEY.DATA_SERIALIZE());
}

static KeyPair GEN_KEY_PAIR(std::string Passcode = "") {

  KeyPair RetVal = {"", ""};

  int SKEY_N = 0;
  int ERRO_N = 1;

  // GENERATING SKEY
  if (Passcode == "") {
    Passcode = GEN_SEED();
  }
  POLYN_MATRIX SKEY = CBDMAP(Passcode, SKEY_N, SMALL_INT, 1, BASE_DIMENTIONS);

  std::string PRIVATE_KEY = (SKEY).DATA_SERIALIZE();

  // GENERATING ERROR
  POLYN_MATRIX ERROR =
      CBDMAP(GEN_SEED(), ERRO_N, SMALL_INT, 1, BASE_DIMENTIONS);

  // COMPUTING PUBLIC AND PRIVATE KEYS
  std::string PUBLIC_KEY = ((base * SKEY) + ERROR).DATA_SERIALIZE();

  RetVal.PublicKey = PUBLIC_KEY;
  RetVal.PrivateKey.Key = PRIVATE_KEY;
  RetVal.PrivateKey.Error = ERROR.DATA_SERIALIZE();

  return (RetVal);
}

static bool ValidatePrivateKey(Private_Key PrivateKey, std::string PublicKey) {
  int N = 1;
  // Gen SKEY and Error
  POLYN_MATRIX SKEY(PrivateKey.Key);
  POLYN_MATRIX ERROR(PrivateKey.Error);

  // ReGen PublicKey
  std::string GIVEN_KEY = PublicKey.substr(SEED_LEN);
  std::string PUBLIC_KEY = ((base * SKEY) + ERROR).DATA_SERIALIZE();

  return (GIVEN_KEY == PUBLIC_KEY);
}

static std::string GenerateSignature(Private_Key PrivateKey, std::string Data) {
  int N = 0;

  // ReGen PrivateKey
  POLYN_MATRIX PRIVATE_KEY(PrivateKey.Key);

  // Generate Error
  POLYN_MATRIX ERROR(PrivateKey.Error);

  // Generate Hasher
  std::string HASHER = (base * ERROR).GET_ITER(0).DATA_SERIALIZE();
  POLYN_MATRIX C = GENERATE_POLYN_MATRIX(Data + HASHER, BASE_DIMENTIONS, 1);

  POLYN_MATRIX Z = ERROR + (C * PRIVATE_KEY);

  std::string Signature = C.DATA_SERIALIZE() + Z.DATA_SERIALIZE();
  std::cout << C.DATA_SERIALIZE().length() << "  "
            << Z.DATA_SERIALIZE().length() << std::endl;
  return (Signature);
}

static bool ValidateSignature(std::string PublicKey, std::string Data,
                              std::string Signature) {
  POLYN_MATRIX C(Signature.substr(0, 780));
  POLYN_MATRIX Z(Signature.substr(780));
  POLYN_MATRIX PUBLIC_KEY(PublicKey);

  std::string HASHER = ((base * Z) - (C * PUBLIC_KEY)).DATA_SERIALIZE();
  if (C.GET_ITER(0) == GENERATE_POLYN_MATRIX(Data + HASHER, 1, 1).GET_ITER(0)) {
    return (true);
  } else {
    return (false);
  }
}
