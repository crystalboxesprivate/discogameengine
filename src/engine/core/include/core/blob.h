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
    return data.get();
  }

  inline usize get_size() {
    return size;
  }

  void allocate(usize in_size, void *init_data = nullptr) {
    assert(!data);
    assert(in_size);
    data = std::make_unique<u8[]>(in_size);//(u8*) malloc(in_size);
    size = in_size;

    if (init_data)
      memcpy(data.get(), init_data, in_size);
  }

  void free() {
    size = 0;
  }

  //void move(Blob &to_blob) {
  //  to_blob.data = data;
  //  to_blob.size = size;
  //  data = nullptr;
  //  size = 0;
  //}

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
  //u8 *data;
  UniquePtr<u8[]> data;
  usize size;
};
