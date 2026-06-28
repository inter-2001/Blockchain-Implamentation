#pragma once
#include "DigitalSigning.hpp"
#include <cryptopp/config_cxx.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <string>

class Wallet {
public:
  std::string PublicKey = "";

  void InstantiateWallet(std::string PrivateKey) {
    PublicKey = Generate_Public_Key(PrivateKey);
  }

  static std::string Generate_Private_Key() {
    PRIVATE_KEY_TYPE PrivateKey;
    PrivateKey.Initialize(rng, CryptoPP::ASN1::secp256r1());
    return (PrivateKeyToString(PrivateKey));
  }

  static std::string Generate_Public_Key(std::string PrivateKey) {
    PUBLIC_KEY_TYPE PublicKey;
    StringToPrivateKey(PrivateKey).MakePublicKey(PublicKey);
    return (PublicKeyToString(PublicKey));
  }

  static bool Verify_Key_Pair(std::string PrivateKey, std::string PublicKey) {
    PUBLIC_KEY_TYPE TempPublicKey;
    try {
      StringToPrivateKey(PrivateKey).MakePublicKey(TempPublicKey);
      if (PublicKeyToString(TempPublicKey) == PublicKey) {
        return (true);
      }
    } catch (...) {
    }

    return (false);
  }

  static std::string Sign_Data(std::string Data, std::string &PrivateKey) {
    SIGNER SignGen(StringToPrivateKey(PrivateKey));
    std::string Signature;

    CryptoPP::StringSource SS1(
        Data, true,
        new CryptoPP::SignerFilter(rng, SignGen,
                                   new CryptoPP::StringSink(Signature)));
    return (Signature);
  }

  static bool Verify_Signature(std::string Data, std::string Signature,
                               std::string PublicKey) {
    VERIFYER Verifier(StringToPublicKey(PublicKey));
    bool Verified = Verifier.VerifyMessage(
        reinterpret_cast<const CryptoPP::byte *>(Data.data()), Data.size(),
        reinterpret_cast<const CryptoPP::byte *>(Signature.data()),
        Signature.size());

    return (Verified);
  }
};
