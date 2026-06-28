#pragma once
#include "Block.hpp"
#include "Transaction.hpp"
#include <cryptopp/config_cxx.h>
#include <ctime>
#include <map>
#include <queue>
#include <stdexcept>
#include <string>

class BlockChain {

  std::vector<Block> Blocks;
  std::priority_queue<Transaction> TransactionPool;

  std::array<Transaction, TransactionsInBlock + 1> GetNextPoolSet() {
    std::array<Transaction, TransactionsInBlock + 1> Set;
    for (int i = 0; i < TransactionsInBlock; i++) {
      Transaction Empty;
      if (!TransactionPool.empty()) {
        Set[i] = TransactionPool.top();
        TransactionPool.pop();
      } else {
        Set[i] = Empty;
      }
    }
    return (Set);
  }

public:
  BlockChain() {
    Block TempBlock;
    TempBlock.GenisisBlock();
    Blocks.push_back(TempBlock);
  }

  void NewBlock(std::string MinerAddress) {
    Block TempBlock;
    TempBlock.ChainBlock(Blocks.back(), GetNextPoolSet(), MinerAddress);
    AddBlock(TempBlock);
  }

  void AddBlock(Block NewBlock) {
    NewBlock.Validate(Blocks.back());
    Blocks.push_back(NewBlock);
  }

  Block Get_Block(int i) {
    if (i >= Blocks.size()) {
      throw(std::out_of_range("index out of bounds"));
    }
    return (Blocks[i]);
  }

  Block Get_Last_Block() { return (Blocks.back()); }

  void PushTransaction(Transaction Datum) {
    if (Datum.Verify()) {
      if (CheckNumericBalance(Datum.GetSender()) >=
          Datum.GetAmmount() + Datum.GetFee()) {
        TransactionPool.push(Datum);
      }
    }
  }

  bool Validate() {
    bool Validated = true;
    for (Block Datum : Blocks) {
      if (Datum.GetIndex() > 0) {
        if (!Datum.Validate(Blocks[Datum.GetIndex() - 1])) {
          Validated = false;
        }
      }
    }
    return (Validated);
  }

  std::map<std::string, Transaction> CheckBalance(std::string WaletKey) {
    std::map<std::string, Transaction> RetMap;
    for (Block Datum : Blocks) {
      std::map<std::string, Transaction> Temp = Datum.CheckBalance(WaletKey);
      RetMap.insert(Temp.begin(), Temp.end());
    }
    return (RetMap);
  }

  double CheckNumericBalance(std::string WaletKey) {
    std::map<std::string, Transaction> Transactions = CheckBalance(WaletKey);
    double RetVal = 0;
    for (auto Datum : Transactions) {
      if (Datum.second.GetSender() == WaletKey) {
        RetVal -= (Datum.second.GetAmmount() + Datum.second.GetFee());
      }
      if (Datum.second.GetReciever() == WaletKey) {
        RetVal += Datum.second.GetAmmount();
      }
    }
    return (RetVal);
  }

  std::string PRINT_CHAIN() {
    std::string RetVal = "";
    for (Block datum : Blocks) {
      RetVal += datum.PrintBlock() + "\n";
    }
    return (RetVal);
  }

  std::string PRINT_POOL() {
    std::string RetVal = "";

    RetVal = "Pending Transactions: " + std::to_string(TransactionPool.size());

    return (RetVal);
  }
};
