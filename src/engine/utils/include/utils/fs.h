#pragma once
#include <core/typedefs.h>

namespace utils {
namespace fs {
bool load_file_to_string(const String &filename, String &contents);
inline String load_file_to_string(const String &filename) {
  String result;
  load_file_to_string(filename, result);
  return result;
}

bool load_file_to_buffer(const String& filename, Vector<u8> &data);
bool save_binary_file(const String&filename, usize size, void*data);

bool create_directory(const String &name);
} // namespace fs
} // namespace utils
