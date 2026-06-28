#pragma once
#include <cstring>
#include <string>
#include <type_traits>

template <typename DATATYPE> std::string SERIALIZE(DATATYPE Var) {
  static_assert(std::is_trivially_copyable_v<DATATYPE>);
  std::string Ret = "";
  unsigned CHARS = sizeof(DATATYPE);
  for (int i = 0; i < CHARS; i++) {
    Ret += (char)(Var >> (8 * i));
  }
  return (Ret);
}

template <typename DATATYPE> DATATYPE DESERIALIZE(std::string Var) {
  static_assert(std::is_trivially_copyable_v<DATATYPE>);
  DATATYPE Ret = 0;
  int CHARS = sizeof(DATATYPE);
  for (int i = 0; i < CHARS; i++) {
    Ret |= static_cast<unsigned char>(Var[i]) << (8 * i);
  }
  return (Ret);
}

/*
template <typename datatype>
std::string serialize_arr(datatype *var, uint32_t size) {
  std::string len = serialize(size);
  std::string datum_size = serialize(static_cast<uint32_t>(sizeof(datatype)));
  std::string retval = "";
  retval += len;
  retval += datum_size;
  if (!std::is_trivially_copyable_v<datatype>) {
    for (int i = 0; i < size; i++) {
      retval += serialize_arr(var[i], sizeof(var[i]) / sizeof(var[i][0]));
    }
  } else {
    for (int i = 0; i < size; i++) {
      retval += serialize<datatype>(var[i]);
    }
  }
  return (retval);
}

template <typename datatype>
std::vector<datatype> deserialize_array(std::string array) {
  std::vector<datatype> retarr;
  unsigned start = 0;
  uint32_t len = deserialize<uint32_t>(array.substr(start, 4));
  start += 4;
  uint32_t datum_size = deserialize<uint32_t>(array.substr(start, 4));
  start += 4;

  if (!std::is_trivially_copyable_v<datatype>) {
    for (int i = 0; i < len; i++) {
      retarr.push_back(deserialize_array<retarr>)
    }
  }
}
*/
