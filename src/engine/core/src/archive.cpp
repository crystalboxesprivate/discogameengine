#include <core/archive.h>
#include <utils/fs.h>

Archive::Archive(const String &filename)
    : type(Type::Loading)
    , current_offset(0) {
  bool loaded = utils::fs::load_file_to_buffer(filename, data_vec);
  assert(loaded);
}

void Archive::serialize(void *data, usize numbytes) {
  switch (type) {
  case Type::Saving: {
    assert(data_vec.capacity() > current_offset);
    memcpy(data_vec.data() + current_offset, data, numbytes);
    current_offset = current_offset + numbytes;
    break;
  }
  case Type::Loading: {
    memcpy(data, data_vec.data() + current_offset, numbytes);
    current_offset += numbytes;
    break;
  }
  case Type::CalculatingSize:
  default: {
    current_offset = current_offset + numbytes;
    break;
  }
  };
}
