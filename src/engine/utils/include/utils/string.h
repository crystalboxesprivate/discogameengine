#pragma once

#include <engine>
#include <sstream>

namespace utils {
namespace string {
// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf/26221725#26221725
#pragma warning(disable : 4996)
template <typename... Args>
String sprintf(const String &format, Args... args) {
  size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1;
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args...);
  return String(buf.get(), buf.get() + size - 1);
}

static bool starts_with(const String &in_string, const String &with_string) {
  usize iteration_count = with_string.size() > in_string.size() ? in_string.size() : with_string.size();
  for (int x = 0; x < iteration_count; x++) {
    if (in_string[x] != with_string[x])
      return false;
  }
  return true;
}

static Vector<char> to_array(String &in_string) {
  Vector<char> out_vector(in_string.size() + 1);
  memcpy(out_vector.data(), in_string.data(), in_string.size());
  out_vector[in_string.size()] = '\0';
  return out_vector;
}

// https://stackoverflow.com/a/3418285
static void replace_in_place(String &in_string, const String &from, const String &to) {
  if (from.empty())
    return;
  size_t start_pos = 0;
  while ((start_pos = in_string.find(from, start_pos)) != std::string::npos) {
    in_string.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

static String replace(const String &in_string, const String &from, const String &to) {
  String copied = in_string;
  replace_in_place(copied, from, to);
  return copied;
}

static String replace(const String &in_string, char from, char to) {
  String copied = in_string;
  for (int x = 0; x < in_string.size(); x++) {
    if (in_string[x] == from)
      copied[x] = to;
  }
  return copied;
}

static String to_lower(const String &in_string) {
  String copied = in_string;
  for (int x = 0; x < copied.size(); x++) {
    putchar(tolower(copied[x]));
  }
  return copied;
}

constexpr usize length(const char *s) {
  return *s ? 1 + length(s + 1) : 0;
}

constexpr usize hash_code(const char *s, usize length) {
  usize h = 0u;
  usize l = length;
  usize i = 0u;
  if (l > 0u)
    while (i < l)
      h = (h << 5) - h + (u8)s[i++];
  return h;
}

constexpr usize hash_code(const char *s) {
  return hash_code(s, length(s));
}

inline usize hash_code(const String &in_string) {
  return hash_code(in_string.c_str(), in_string.size());
}

static void parse_into_lines(const String &input, Vector<String> &out_arr) {
  std::stringstream ss(input);
  std::string to;

  if (input.size()) {
    while (std::getline(ss, to, '\n')) {
      out_arr.push_back(to);
    }
  }
}
} // namespace string
} // namespace utils
