#pragma once

#include <string.h>
#include <string>
#include <vector>

#include <core/math/math.h>
#include <core/typedefs.h>

#define m_serialize_basic_type(type_name)                                                                              \
  inline friend Archive &operator<<(Archive &archive, type_name &value) {                                              \
    archive.serialize(&value, sizeof(type_name));                                                                      \
    return archive;                                                                                                    \
  }

struct Archive {
  enum class Type : u8 { CalculatingSize, Loading, Saving };

  Archive()
      : type(Type::CalculatingSize)
      , current_offset(0) {
  }

  Archive(usize capacity)
      : type(Type::Saving)
      , current_offset(0) {
    data_vec.reserve(capacity);
  }

  Archive(const String &filename);

  void *data() {
    return data_vec.data();
  }

  usize get_size() {
    return current_offset;
  }

  bool is_saving() {
    return type == Type::Saving;
  }
  bool is_loading() {
    return type == Type::Loading;
  }

  m_serialize_basic_type(u8);
  m_serialize_basic_type(u16);
  m_serialize_basic_type(u32);
  m_serialize_basic_type(u64);

  m_serialize_basic_type(i8);
  m_serialize_basic_type(i16);
  m_serialize_basic_type(i32);
  m_serialize_basic_type(i64);

  m_serialize_basic_type(f32);
  m_serialize_basic_type(f64);

  m_serialize_basic_type(bool);

  inline friend Archive &operator<<(Archive &archive, const char *value) {
    archive.serialize((void *)value, strlen(value));
    return archive;
  }

  inline friend Archive &operator<<(Archive &archive, String &value) {
    usize string_size = value.size();
    archive << string_size;
    if (archive.is_loading()) {
      value.resize(string_size);
    }
    archive.serialize((void *)value.data(), value.size());
    return archive;
  }

  inline friend Archive &operator<<(Archive &archive, glm::vec3 &value) {
    archive << value[0];
    archive << value[1];
    archive << value[2];
    return archive;
  }

  inline friend Archive &operator<<(Archive &archive, glm::vec4 &value) {
    archive << value[0];
    archive << value[1];
    archive << value[2];
    archive << value[3];
    return archive;
  }

  inline friend Archive &operator<<(Archive &archive, glm::quat &value) {
    archive << value[0];
    archive << value[1];
    archive << value[2];
    archive << value[3];
    return archive;
  }

  template <typename T>
  inline friend Archive &operator<<(Archive &archive, Vector<T> &value) {
    usize vector_size = value.size();
    archive << vector_size;

    if (archive.is_loading()) {
      value.resize(vector_size);
    }
    for (auto &item : value) {
      archive << item;
    }
    return archive;
  }

  template <typename K, typename V>
  inline friend Archive &operator<<(Archive &archive, HashMap<K, V> &hash_map) {
    usize vector_size = hash_map.size();
    archive << vector_size;

    if (archive.is_loading()) {
      for (int x = 0; x < vector_size; x++) {
        K key;
        V value;
        archive << key;
        archive << value;
        hash_map[key] = value;
      }
    } else {
      for (auto &item : hash_map) {
        K key = item.first;
        V &value = item.second;
        archive << key;
        archive << value;
      }
    }
    return archive;
  }

  inline usize get_current_offset() {
    return current_offset;
  }

  void serialize(void *data, usize numbytes);

private:
  Type type;

  usize current_offset;
  Vector<u8> data_vec;
};
