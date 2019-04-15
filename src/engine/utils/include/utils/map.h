#pragma once

#define map_has(map, key) [map, key]() { return map.find(key) == map.end(); }()
#define map_find(map, key)                                                                                             \
  [map, key]() {                                                                                                       \
    auto it = map.find(key);                                                                                           \
    if (it != map.end())                                                                                               \
      return &it->second;                                                                                              \
    auto it_ = &map.begin()->second;                                                                                   \
    it_ = nullptr;                                                                                                     \
    return it_;                                                                                                        \
  }()
