#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <map>
#include <vector>
#include <list>

typedef size_t usize;

typedef unsigned char u8;
typedef char i8;

typedef unsigned short u16;
typedef short i16;

typedef unsigned int u32;
typedef int i32;

typedef unsigned long long u64;
typedef long long i64;

typedef float f32;
typedef double f64;

typedef std::string String;
template <class T>
using SharedPtr = std::shared_ptr<T>;
template <class T>
using UniquePtr = std::unique_ptr<T>;
template <class T>
using List = std::list<T>;
template <class T>
using Vector = std::vector<T>;
template <class K, class V>
using Map = std::map<K, V>;
template <class K, class V>
using HashMap = std::unordered_map<K, V>;
