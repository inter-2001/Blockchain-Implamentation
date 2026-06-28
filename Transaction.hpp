#pragma once
#include "Utils.hpp"
#include "Wallet.hpp"
#include <chrono>
#include <ctime>
#include <string>

static unsigned short Selection = 0;

class Transaction {
  bool Initalised = false;
  std::string Hash = "";
  std::string Signature = "";
  std::string SenderAddress = "";
  std::string RecipientAddress = "";
  std::chrono::time_point<std::chrono::system_clock> Timestamp;
  double Ammount = 0;
  double Fee = 0;

public:
  void Init(std::string &SenderPC, std::string Sender, std::string Reciever,
            double ammount, double fee = 0) {
    Initalised = true;
    SenderAddress = Sender;
    RecipientAddress = Reciever;
    Ammount = ammount;
    Fee = fee;
    Timestamp = std::chrono::system_clock::now();
    Hash = HASH_STRING(SenderAddress + RecipientAddress +
                       std::to_string(Ammount) + std::to_string(Fee) +
                       std::format("{:%d-%m-%Y %H:%M:%OS}", Timestamp));
    if (SenderPC == "RewardVector") {
      Signature = "N/A";
    } else {
      Signature = Wallet::Sign_Data(Hash, SenderPC);
    }
  }

  bool Is_Initalised() { return (Initalised); }

  bool GetFee() { return (Fee); }

  std::string GetHash() { return (Hash); }
  std::string GetReciever() { return (RecipientAddress); }
  std::string GetSender() { return (SenderAddress); }
  double GetAmmount() { return (Ammount); }

  std::string PRINT_TRANSACTION() {
    std::string RetVal = "";
    RetVal +=
        "Timestamp: " + std::format("{:%d-%m-%Y %H:%M:%OS}", Timestamp) + "\n";
    RetVal += "Sender   : " + SenderAddress + "\n";
    RetVal += "Reciever : " + RecipientAddress + "\n";
    RetVal += "Ammount  : " + std::to_string(Ammount) +
              " Fee: " + std::to_string(Fee) +
              " Reward: " + std::to_string(CalculateReward()) + "\n";
    RetVal += "Hash:\n" + StringToHex(Hash) + "\n";
    RetVal += "Signature:\n" + StringToHex(Signature) + "\n";

    return (RetVal);
  }

  bool Verify() {
    if (SenderAddress == "Rewards" && Signature == "N/A") {
      return (true);
    }
    return (Wallet::Verify_Signature(Hash, Signature, SenderAddress));
  }

  // the longer the transaction waits for processing the higher the reward to
  // process it alongside the fee for processing
  double CalculateReward() const {
    double Reward = 0;
    double TimeDiferance = std::chrono::duration_cast<std::chrono::seconds>(
                               std::chrono::system_clock::now() - Timestamp)
                               .count();
    Reward = (TimeDiferance * (Ammount / 1000));
    Reward += Fee;
    return (Reward);
  }

  // todo
  // add the other selections
  const bool operator>(const Transaction &other) const {
    if (Selection == 0) {
      if (CalculateReward() > other.CalculateReward()) {
        return (true);
      }
    }
    return (false);
  }

  const bool operator<(const Transaction &other) const {
    if (Selection == 0) {
      if ((CalculateReward() < other.CalculateReward())) {
        return (true);
      }
    }
    return (false);
  }
};
