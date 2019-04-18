#pragma once

#include <core/typedefs.h>
#include <core/archive.h>

struct Blob {
  Blob()
      : size(0) {
  }

  Blob(const Blob &blob) {
    if (blob.data.size()) {
      allocate(blob.get_size(), blob.get_data());
    }
  }

  Blob &operator=(const Blob &blob) {
    if (blob.data.size()) {
      allocate(blob.get_size(), blob.get_data());
    }
    return *this;
  }

  inline const u8 *get_data() const {
    return data.data();
  }

  inline u8 *get_data() {
    return data.data();
  }

  inline usize get_size() const {
    return size;
  }

  void allocate(usize in_size, const void *init_data = nullptr) {
    assert(!data.size());
    assert(in_size);
    data.resize(in_size);
    size = in_size;

    if (init_data)
      memcpy(data.data(), init_data, in_size);
  }

  void free() {
    data.clear();
    data.shrink_to_fit();
    size = 0;
  }
  inline friend Archive &operator<<(Archive &archive, Blob &blob) {
    archive << blob.size;
    if (archive.is_loading()) {
      if (blob.size) {
        blob.allocate(blob.size);
      }
    }
    archive.serialize(blob.get_data(), blob.get_size());
    return archive;
  }

private:
  // u8 *data;
  Vector<u8> data;
  usize size;
};
