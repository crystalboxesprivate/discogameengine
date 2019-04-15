#pragma once

#include <bitset>
#include <core/archive.h>

struct Guid {
  unsigned int a, b, c, d;
  Guid() : a(0), b(0), c(0), d(0) {}

  static Guid make_new();
  bool is_valid() const { return (a | b | c | d) != 0; }

  const unsigned int &operator[](int index) const;

  friend bool operator==(const Guid &lhs, const Guid &rhs) {
    return ((lhs.a ^ rhs.a) | (lhs.b ^ rhs.b) | (lhs.c ^ rhs.c) | (lhs.d ^ rhs.d)) == 0;
  }

  friend bool operator!=(const Guid &lhs, const Guid &rhs) {
    return ((lhs.a ^ rhs.a) | (lhs.b ^ rhs.b) | (lhs.c ^ rhs.c) | (lhs.d ^ rhs.d)) != 0;
  }

  std::string to_string() const {
    char buff[128];
    sprintf(buff, "%d-%d-%d-%d", a, b, c, d);
    return buff;
  }

  size_t get_hash() const;
  friend Archive& operator<<(Archive& archive, Guid& guid);
};

namespace std {
template <> struct hash<Guid> {
  size_t operator()(const Guid &value) const { return value.get_hash(); }
};
} // namespace std
