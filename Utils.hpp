#pragma once
#include "cthash/sha2/sha256.hpp"
#include <iomanip>
#include <pthread.h>
#include <sstream>
#include <string>

#define ThreadCount 4

static bool MINE_FINISHED = false;

static std::string StringToHex(std::string Data) {
  std::stringstream RetVal;
  RetVal << std::hex << std::setfill('0');
  for (unsigned short Datum : Data) {
    RetVal << std::setw(2) << Datum;
  }
  return (RetVal.str());
}

static std::string HASH_STRING(const std::string &Data) {
  cthash::sha256 HASHER;
  HASHER.update(Data);
  std::string RetVal = "";
  auto RetHash = HASHER.final();
  for (auto Datum : RetHash) {
    RetVal += (char)Datum;
  }
  return (RetVal);
}

static std::string GetMinedHash(const std::string &Data,
                                const unsigned &NONCE) {
  cthash::sha256 HASHER;
  HASHER.update(Data);
  HASHER.update(std::to_string(NONCE));
  std::string RetVal = "";
  auto RetHash = HASHER.final();
  for (auto Datum : RetHash) {
    RetVal += (char)Datum;
  }
  return (RetVal);
}

static bool MINE_HASH(cthash::sha256 HASHER, unsigned NONCE,
                      std::string Check) {
  std::string RetVal = "";
  HASHER.update(std::to_string(NONCE));

  auto RetHash = HASHER.final();
  for (auto Datum : RetHash) {
    RetVal += (char)Datum;
  }
  if (StringToHex(RetVal).substr(0, Check.length()) == Check) {
    std::cerr << StringToHex(RetVal).substr(0, Check.length()) << "  " << Check
              << "\n";
    return true;
  } else {
    return false;
  }
}

static unsigned THREADED_MINE_HANDLER(cthash::sha256 HASHER, unsigned NONCE,
                                      std::string Check) {
  while (!MINE_HASH(HASHER, NONCE, Check) && !MINE_FINISHED) {
    NONCE += ThreadCount - 1;
  }
  return (NONCE);
}
