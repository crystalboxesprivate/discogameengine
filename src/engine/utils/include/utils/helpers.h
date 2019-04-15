#pragma once

#include <unordered_map>

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
