#pragma once

#include <engine>

namespace utils {
namespace path {
String join(const String &a, const String &b);
bool exists(const String&path);
String filename(const String& in_path, bool keep_extension = true);

static void splitext(const String &in_filename, String &file, String &ext) {
  usize found = in_filename.find_last_of(".");
  file = in_filename.substr(0, found);
  ext = in_filename.substr(found + 1);
}

static String get_extension(const String &in_path) {
  String file = filename(in_path);
  String out_file, out_ext;
  splitext(file, out_file, out_ext);
  return out_ext;
}

} // namespace path
} // namespace utils
