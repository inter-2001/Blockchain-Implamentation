#include "CRISALIS/GENERIC.hpp"

int main() {

  KeyPair Keys = GEN_KEY_PAIR();
  std::string Data = "Hey, this is working!";
  std::string Signature = GenerateSignature(Keys.PrivateKey, Data);

  if (ValidateSignature(Keys.PublicKey, Data, Signature)) {
    std::cout << "WORKING" << std::endl;
  } else {
    std::cout << "FAIL" << std::endl;
  }

  return (0);
}
