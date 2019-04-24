#pragma once

#include <unordered_map>
#include <vector>

namespace helpers {
template <typename K, typename T>
T *find(std::unordered_map<K, T> &map, const K &key) {
  typename std::unordered_map<K, T>::iterator it = map.find(key);
  if (it == map.end())
    return nullptr;
  return &it->second;
}

template <class T>
T rand01() {
  return (T)((double)rand() / RAND_MAX);
}

} // namespace helpers
namespace utils {
template <typename T>
void free_vector(std::vector<T> &vec) {
  vec.clear();
  vec.shrink_to_fit();
}
} // namespace utils
