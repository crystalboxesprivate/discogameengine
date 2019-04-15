#pragma once

#include <core/typedefs.h>
#include <core/archive.h>

struct Blob {
  Blob()
      : data(nullptr)
      , size(0) {
  }

  ~Blob() {
    free();
  }

  inline u8 *get_data() {
    return data;
  }

  inline usize get_size() {
    return size;
  }

  void allocate(usize in_size, void *init_data = nullptr) {
    assert(!data);
    assert(in_size);
    data = (u8*) malloc(in_size);
    size = in_size;

    if (init_data)
      memcpy(data, init_data, in_size);
  }

  void free() {
    if (data)
      ::free(data);
    size = 0;
  }

  void move(Blob &to_blob) {
    to_blob.data = data;
    to_blob.size = size;
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
    archive.serialize(blob.data, blob.size);
    return archive;
  }

private:
  u8 *data;
  usize size;
};
