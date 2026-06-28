#pragma once
#include "SERIALIZER.hpp"
#include "Transaction.hpp"
#include "Utils.hpp"
#include "cthash/sha2/sha256.hpp"
#include <array>
#include <cryptopp/config_cxx.h>
#include <cstddef>
#include <ctime>
#include <future>
#include <map>
#include <string>
#define TransactionsInBlock 6

static unsigned Dificulty_Threashold = 4;

static bool MULTITHREAD = true;

static std::string MergeHashes(std::string Hash1, std::string Hash2) {
  return (HASH_STRING(Hash1 + Hash2));
}

class Block {
  unsigned Index = 0;
  unsigned Timestamp = 0;
  std::array<Transaction, TransactionsInBlock + 1> Transactions;
  unsigned TransInBlock = 0;
  std::string HASH = "";
  std::string HASH_PREV = "";
  unsigned Nonce = 0;

public:
  Block() = default;
  Block(const Block &other) = default;

  unsigned GetIndex() { return (Index); }

  void ChainBlock(Block &Previous,
                  std::array<Transaction, TransactionsInBlock + 1> Data,
                  std::string MinerAddress) {
    HASH_PREV = Previous.HASH;
    Index = Previous.Index + 1;
    Timestamp = std::time(NULL);
    Transactions = Data;
    double Reward = 0;
    for (int i = 0; i < TransactionsInBlock; i++) {
      if (Transactions[i].Is_Initalised()) {
        TransInBlock++;
        Reward += Transactions[i].CalculateReward();
      }
    }
    std::string Sender = "RewardVector";

    // Dificulty_Threashold = (TransactionsInBlock - TransInBlock) + 2;
    //  Commented out for testing

    // Gives 1 coin/Index for each empy block as well as increadsing
    // dificulty for each blank entry

    if (TransInBlock == 0) {
      Reward = (double)1 / Index;
    }
    // Add Reward Transaction to uninitilised final transaction
    Transactions[TransactionsInBlock].Init(Sender, "Rewards", MinerAddress,
                                           Reward);

    HASH =
        Mine(HASH_PREV + SERIALIZE(Index) + SERIALIZE(Timestamp) + MerkleRoot(),
             Nonce);
  }

  std::string MerkleRoot() {
    bool Combined = false;
    std::vector<std::string> Hashes;
    for (int i = 0; i < TransactionsInBlock + 1; i++) {
      Hashes.push_back(Transactions[i].GetHash());
    }
    while (Hashes.size() > 1) {
      std::vector<std::string> CombinedHashes;
      for (int i = 1; i < Hashes.size(); i += 2) {
        CombinedHashes.push_back(MergeHashes(Hashes[i - 1], Hashes[i]));
      }
      if (Hashes.size() % 2) {
        CombinedHashes.push_back(Hashes.back());
      }
      Hashes = CombinedHashes;
    }
    return (Hashes.back());
  }

  static std::string Mine(std::string DataToHash, unsigned &nonce) {
    std::cerr << "STARTING MINING PROCESS" << "\n";
    std::string Check = "";
    std::string RetVal = "";
    for (int i = 0; i < Dificulty_Threashold; i++) {
      Check += "0";
      RetVal += "1";
    }
    if (MULTITHREAD) {
      MINE_FINISHED = false;
      std::array<std::future<unsigned>, ThreadCount> ReturnValues;
      cthash::sha256 Hasher;
      Hasher.update(DataToHash);
      RetVal = "";
      for (int i = 0; i < ThreadCount; i++) {
        ReturnValues[i] =
            std::async(THREADED_MINE_HANDLER, Hasher, nonce + i, Check);
      }
      while (true) {
        for (int i = 0; i < ThreadCount; i++) {
          if (ReturnValues[i].wait_for(std::chrono::seconds(0)) ==
              std::future_status::ready) {
            nonce = ReturnValues[i].get();
            MINE_FINISHED = true;
            return (GetMinedHash(DataToHash, nonce));
          }
        }
      }
    } else {
      while (StringToHex(RetVal).substr(0, Dificulty_Threashold) != Check) {
        RetVal = GetMinedHash(DataToHash, nonce);
        nonce++;
      }
      nonce -= 1;
    }
    return (RetVal);
  }

  void GenisisBlock() {
    HASH_PREV = "";
    Index = 0;
    Timestamp = std::time(NULL);
    HASH = HASH_STRING(HASH_PREV + SERIALIZE(Index) + SERIALIZE(Timestamp));
  }

  // Validates that the previous hash matches and that the hash matches the data
  // if requested
  bool Validate(Block &Previous, bool DataCheck = true) {
    if (DataCheck) {
      std::string TempHash =

          HASH_STRING(HASH_PREV + SERIALIZE(Index) + SERIALIZE(Timestamp) +
                      MerkleRoot() + std::to_string(Nonce));
      if (HASH != TempHash) {
        return (false);
      }
    }
    if (Previous.HASH == HASH_PREV) {
      return (true);
    }
    return (false);
  }

  std::string PrintBlockTransactions() {
    std::string RetVal = "";
    for (int i = 0; i < TransInBlock; i++) {
      RetVal += Transactions[i].PRINT_TRANSACTION() + "\n";
    }
    RetVal += "REWARD_TRANSACTION\n" +
              Transactions[TransactionsInBlock].PRINT_TRANSACTION();
    return (RetVal);
  }

  std::string PrintBlock() {
    std::string RetVal = "\n\nBlock at Index: " + std::to_string(Index) +
                         "\nTimestamp: " + std::to_string(Timestamp) + "\n";
    RetVal += "nonce    : " + std::to_string(Nonce) + "\n";
    RetVal += "Dificulty: " + std::to_string(Dificulty_Threashold) + "\n";
    RetVal += "Hash     : " + StringToHex(HASH) + "\n";
    RetVal += "PrevHash : " + StringToHex(HASH_PREV) + "\n";
    RetVal += "Datums   : " + std::to_string(TransInBlock) + "\n";
    if (Index > 0) {
      RetVal += "Data     : \n\n" + PrintBlockTransactions() + "\n";
    }
    return (RetVal);
  }

  std::map<std::string, Transaction> CheckBalance(std::string WaletKey) {
    std::map<std::string, Transaction> RetMap;
    for (int i = 0; i < TransactionsInBlock + 1; i++) {
      if (Transactions[i].Is_Initalised()) {
        if (WaletKey == Transactions[i].GetReciever() ||
            WaletKey == Transactions[i].GetSender()) {
          RetMap[Transactions[i].GetHash()] = Transactions[i];
        }
      }
    }
    return (RetMap);
  }
};
