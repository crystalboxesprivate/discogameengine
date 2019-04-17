#pragma once

#include <core/typedefs.h>
#include <core/archive.h>

struct Blob {
  Blob()
      : data(nullptr)
      , size(0) {
  }

  Blob(const Blob &blob) {
    if (blob.data) {
      allocate(blob.get_size(), blob.get_data());
    }
  }
  Blob &operator=(const Blob &blob) {
    if (blob.data) {
      allocate(blob.get_size(), blob.get_data());
    }
    return *this;
  }

  inline u8 *get_data() const {
    return data.get();
  }

  inline usize get_size() const {
    return size;
  }

  void allocate(usize in_size, void *init_data = nullptr) {
    assert(!data);
    assert(in_size);
    data = std::make_unique<u8[]>(in_size);
    size = in_size;

    if (init_data)
      memcpy(data.get(), init_data, in_size);
  }

  void free() {
    data = nullptr;
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
  UniquePtr<u8[]> data;
  usize size;
};
