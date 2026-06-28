#pragma once
#include "CRISALIS.hpp"
#include "CRYSALIS_DEFS.hpp"
#include "MATRIX.hpp"
#include "POLYN.hpp"
#include "SERIALIZER.hpp"
#include "plusaes.hpp"
#include <cstddef>
#include <cstdint>
#include <linux/limits.h>
#include <ostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ kyber implementation class

namespace KRYSALIS {

typedef std::string DATA_ARAY;

class KYBER {

public:
  POLYN FKEY;
  std::vector<unsigned char> AES_KEY;
  POLYN_MATRIX SKEY;
  std::string FALLBACK_KEY;
  unsigned INITIATOR_PACKET_SIZE;

  // Implementation Variables

  typedef std::string TRANSMISSION_CONTAINER;
  unsigned char BASE_DIMENTIONS;

  TRANSMISSION_CONTAINER InitiatorVal;

public:
  KYBER() {}

  bool CONNECTED = false;
  void operator=(KYBER &OTHER) { FKEY = OTHER.FKEY; }
  bool KEY_RETREVED_SUCESS = true;

  /*Handshake functions*/

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ INITIATION FUNCTION ~~
  // Dispersal of shared information.

  TRANSMISSION_CONTAINER
  INITIATOR(char K) { // K sets the dimensions of the base matrix

    BASE_DIMENTIONS = K;

    int N = 0;
    DATA_ARAY BASE_SEED = GEN_SEED();

    // DEBUG STATEMENT
    if constexpr (DEBUG) {
      PRINT_BIT_STRING(BASE_SEED);
      std::cout << "GENRATING SKEY" << std::endl;
    }

    // GENERATING SKEY
    SKEY = CBDMAP(GEN_SEED(), N, SMALL_INT, 1, BASE_DIMENTIONS);

    if constexpr (DEBUG) {
      std::cout << "GENERATING BASE" << std::endl;
    }

    // GENERATING BASE
    POLYN_MATRIX base =
        GENERATE_POLYN_MATRIX(BASE_SEED, BASE_DIMENTIONS, BASE_DIMENTIONS);

    if constexpr (DEBUG) {
      std::cout << "GENERATING ERROR" << std::endl;
    }
    // GENERATING ERROR
    POLYN_MATRIX ERROR = CBDMAP(GEN_SEED(), N, SMALL_INT, 1, BASE_DIMENTIONS);

    TRANSMISSION_CONTAINER TRANSMISION_ARRAY;

    // COMPUTING TRANSMISION KEY
    if constexpr (DEBUG) {
      std::cout << "GENERATING KEYS" << std::endl;
    }
    DATA_ARAY TRANSMISION_KEY = ((base * SKEY) + ERROR).DATA_SERIALIZE();

    TRANSMISION_ARRAY += BASE_DIMENTIONS;                   // [0]
    TRANSMISION_ARRAY += (unsigned char)BASE_SEED.length(); // [1]
    TRANSMISION_ARRAY +=
        (unsigned char)(TRANSMISION_KEY.length() / UINT8_MAX); // [2]
    TRANSMISION_ARRAY +=
        (unsigned char)(TRANSMISION_KEY.length() %
                        UINT8_MAX); // [3]    const std::vector<unsigned char>
                                    // key =

    TRANSMISION_ARRAY += BASE_SEED;
    TRANSMISION_ARRAY += TRANSMISION_KEY;
    InitiatorVal = TRANSMISION_ARRAY;

    if constexpr (DEBUG) {
      std::cout << "\nINITIATON COMPLETE" << std::endl;
    }

    return (TRANSMISION_ARRAY);
  }

  // ~~~~~~~~~~~~~~~ GENRATE AND ENCAPSULATE KEY BASED ON INITIAROR DATA ~~
  // This is the first response from the reciever to the sender
  // KYBER ALGORITHM 5

  TRANSMISSION_CONTAINER
  ENCAPSULATE(TRANSMISSION_CONTAINER &Public_Key,
              DATA_ARAY SharedKey = GEN_SEED()) {

    int N = 0;
    CONNECTED = true;
    BASE_DIMENTIONS = (unsigned char)Public_Key[0];
    BASEDATATYPE BaseSeedLen = (unsigned char)Public_Key[1];
    BASEDATATYPE TransKeyLen = (unsigned char)Public_Key[2] * UINT8_MAX;
    TransKeyLen += (unsigned char)Public_Key[3];

    DATA_ARAY BASE_SEED = Public_Key.substr(4, BaseSeedLen);

    if constexpr (DEBUG) {
      std::cout << "BASE SEED" << std::endl;
      PRINT_BIT_STRING(BASE_SEED);
    }

    DATA_ARAY TRANSMISION_KEY = Public_Key.substr(4 + BaseSeedLen, TransKeyLen);

    // GENERATE KEY DATA

    POLYN_MATRIX Partial_Key = POLYN_MATRIX(TRANSMISION_KEY);

    // GENERATING SKEY
    SKEY = CBDMAP(SharedKey, N, SMALL_INT, BASE_DIMENTIONS, 1);

    // COMPUTING FKEY
    FKEY = (SKEY * Partial_Key).GetVal(0, 0);

    // COMPUTING BASE
    POLYN_MATRIX base =
        GENERATE_POLYN_MATRIX(BASE_SEED, BASE_DIMENTIONS, BASE_DIMENTIONS);

    // GENERATING ERROR
    POLYN_MATRIX ERROR = CBDMAP(SharedKey, N, SMALL_INT, BASE_DIMENTIONS, 1);

    // GENERATING ERROR1
    POLYN ERROR1 = CBDMAP(SharedKey, N, SMALL_INT - 1, 1, 1).GetVal(0, 0);

    // ENCAPSULATE TRANSMISION

    TRANSMISSION_CONTAINER TRANSMISION_ARRAY = "";

    // U is a BASE_DIMENTIONS x 1 POLYN_MATRIX
    // COMPUTING U
    if constexpr (DEBUG) {
      std::cout << "GENERATE U" << std::endl;
    }
    POLYN_MATRIX U = ((SKEY * base) + ERROR);

    DATA_ARAY U_S = U.DATA_SERIALIZE();

    // COMPUTING V
    POLYN V = ((FKEY + ERROR1) + ENCODE_TO_POLLY(SharedKey));
    DATA_ARAY V_S = V.DATA_SERIALIZE();

    // Finishing off the Unhashed_Fkey
    TRANSMISION_ARRAY += (unsigned char)(U_S.length() / UINT8_MAX);
    TRANSMISION_ARRAY += (unsigned char)(U_S.length() % UINT8_MAX);
    TRANSMISION_ARRAY += (unsigned char)(V_S.length() / UINT8_MAX);
    TRANSMISION_ARRAY += (unsigned char)(V_S.length() % UINT8_MAX);
    TRANSMISION_ARRAY += U_S + V_S;

    // GENERATING FKEY
    AES_KEY = GEN_AES_KEY(TRANSMISION_ARRAY + SharedKey);
    return (TRANSMISION_ARRAY);
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~ DECAPSULATE ENCAPSULATED KEY ~~
  // This is the final response from the sender before encrypted messaging
  // between the sender and reciever
  //
  void DECAPSULATE(
      TRANSMISSION_CONTAINER &EncapsulatedKey) { // initiator runs this on

    FALLBACK_KEY = GEN_SEED();

    DATA_ARAY Unhashed_Fkey = EncapsulatedKey;

    CONNECTED = true;

    unsigned START = 4;

    // Retrive U
    unsigned ULEN = (unsigned char)EncapsulatedKey[0] * UINT8_MAX;
    ULEN += (unsigned char)EncapsulatedKey[1];
    DATA_ARAY u = EncapsulatedKey.substr(START, ULEN);
    POLYN_MATRIX U = POLYN_MATRIX(u);
    FKEY = (U * SKEY).GetVal(0, 0);

    DATA_ARAY FULLKEY = DECODE_FROM_POLLY<DATA_ARAY>(FKEY);

    // Retrive V
    unsigned VLEN = (unsigned char)EncapsulatedKey[2] * UINT8_MAX;
    VLEN += (unsigned char)EncapsulatedKey[3];
    DATA_ARAY v = EncapsulatedKey.substr(START + ULEN, VLEN);
    POLYN V = POLYN(v);

    // response function body:
    DATA_ARAY SharedKey = DECODE_FROM_POLLY<DATA_ARAY>(V - FKEY);

    if (!CTC(EncapsulatedKey, ENCAPSULATE(InitiatorVal, SharedKey))) {
      SharedKey = FALLBACK_KEY;

      // DEBUG
      if constexpr (DEBUG) {
        std::cout << "\n\nSHARED_KEY_DECODING_FAILURE" << std::endl;
        std::cout << SharedKey << std::endl;
      }

      KEY_RETREVED_SUCESS = false;
    } else {
      SharedKey = SharedKey;
      KEY_RETREVED_SUCESS = true;
    }
    // Finishing off the Unhashed_Fkey

    // GENERATING FKEY
    AES_KEY = GEN_AES_KEY(EncapsulatedKey + SharedKey);
  }

  POLYN FULLKEY() { return (FKEY); }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Encrypt and Decrypt functions ~~

  DATA_ARAY ENCRYPT(std::string base_data) {
    std::string cypher = "";

    unsigned char iv[16];

    std::string iv_base = GEN_AES_VECTOR();

    for (int i = 0; i < 16; i++) {
      iv[i] = (unsigned char)iv_base[i];
    }

    const uint32_t data_size =
        plusaes::get_padded_encrypted_size(base_data.size());

    std::vector<unsigned char> encrypted_data(data_size);

    plusaes::encrypt_cbc((unsigned char *)base_data.data(), base_data.size(),
                         &AES_KEY[0], AES_KEY.size(), &iv, &encrypted_data[0],
                         encrypted_data.size(), true);

    for (auto datum : encrypted_data) {
      cypher += ((char)datum);
    }

    std::string Len = SERIALIZE(data_size);

    return (Len + iv_base + cypher);
  }

  DATA_ARAY DECRYPT(DATA_ARAY cypher_data) {

    unsigned long padded_size = 0;
    uint32_t data_size = DESERIALIZE<uint32_t>(cypher_data.substr(0, 4));
    std::string iv_base = cypher_data.substr(4, 16);
    std::string cypher = cypher_data.substr(20);

    unsigned char iv[16];

    for (int i = 0; i < 16; i++) {
      iv[i] = (unsigned char)iv_base[i];
    }

    std::vector<unsigned char> decrypted_data(data_size);

    plusaes::decrypt_cbc((unsigned char *)cypher.data(), cypher.size(),
                         &AES_KEY[0], AES_KEY.size(), &iv, &decrypted_data[0],
                         decrypted_data.size(), &padded_size);

    std::string mess = "";
    for (auto datum : decrypted_data) {
      mess += ((char)datum);
    }
    return (mess);
  }
};
} // namespace KRYSALIS
