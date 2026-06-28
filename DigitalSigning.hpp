#pragma once
#include <cryptopp/base64.h>
#include <cryptopp/basecode.h>
#include <cryptopp/config_int.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/ecp.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/oids.h>
#include <cryptopp/osrng.h>
#include <cryptopp/queue.h>
#include <cryptopp/sha.h>

static CryptoPP::AutoSeededRandomPool rng;

// Type Definitions

#define PRIVATE_KEY_TYPE                                                       \
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey

#define PUBLIC_KEY_TYPE                                                        \
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey

#define SIGNER CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Signer

#define VERIFYER CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Verifier

// Input and output Keys

static std::string PublicKeyToString(const PUBLIC_KEY_TYPE &PublicKey) {
  std::string RetVal = "";

  const CryptoPP::ECP::Point &Q = PublicKey.GetPublicElement();

  CryptoPP::SecByteBlock Raw(64);

  Q.x.Encode(Raw, 32);
  Q.y.Encode(Raw + 32, 32);

  CryptoPP::Base64Encoder Enc(new CryptoPP::StringSink(RetVal), false);

  Enc.Put(Raw, Raw.size());
  Enc.MessageEnd();

  return RetVal;
}

static PUBLIC_KEY_TYPE StringToPublicKey(const std::string &STRPublicKey) {

  PUBLIC_KEY_TYPE PublicKey;

  std::string Decoded;

  CryptoPP::StringSource Data(
      STRPublicKey, true,
      new CryptoPP::Base64Decoder(new CryptoPP::StringSink(Decoded)));

  if (Decoded.size() != 64)
    throw std::runtime_error("Invalid public key size");

  CryptoPP::Integer QX(reinterpret_cast<const CryptoPP::byte *>(Decoded.data()),
                       32);

  CryptoPP::Integer QY(
      reinterpret_cast<const CryptoPP::byte *>(Decoded.data()) + 32, 32);

  CryptoPP::ECP::Point Q(QX, QY);

  PublicKey.Initialize(CryptoPP::ASN1::secp256r1(), Q);

  return PublicKey;
}

static std::string PrivateKeyToString(const PRIVATE_KEY_TYPE &PrivateKey) {

  std::string RetVal = "";

  const CryptoPP::Integer &X = PrivateKey.GetPrivateExponent();

  CryptoPP::SecByteBlock Raw(32);

  X.Encode(Raw, Raw.size());

  CryptoPP::Base64Encoder Enc(new CryptoPP::StringSink(RetVal), false);

  Enc.Put(Raw, Raw.size());
  Enc.MessageEnd();

  return RetVal;
}

static PRIVATE_KEY_TYPE StringToPrivateKey(const std::string &STRPrivateKey) {

  PRIVATE_KEY_TYPE PrivateKey;

  std::string Decoded;

  CryptoPP::StringSource Data(
      STRPrivateKey, true,
      new CryptoPP::Base64Decoder(new CryptoPP::StringSink(Decoded)));

  if (Decoded.size() != 32)
    throw std::runtime_error("Invalid private key size");

  CryptoPP::Integer X(reinterpret_cast<const CryptoPP::byte *>(Decoded.data()),
                      Decoded.size());

  PrivateKey.Initialize(CryptoPP::ASN1::secp256r1(), X);

  return PrivateKey;
}
