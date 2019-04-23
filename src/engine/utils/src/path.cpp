#include <utils/path.h>
#include <utils/helpers.h>
#include <utils/string.h>
#include <cassert>
#include <fstream>

namespace utils {
namespace path {

String join(const String &a, const String &b) {
  return a + "/" + b;
}
bool exists(const String &path) {
  return std::ifstream(path).good();
}

String filename(const String &in_path, bool keep_extension) {
  usize found = in_path.find_last_of("/\\");
  String filename = in_path.substr(found + 1);
  if (!keep_extension) {
    String out_file, out_ext;
    splitext(filename, out_file, out_ext);
    return out_file;
  }
  return filename;
}

String parent(const String &in_path) {
  usize found = in_path.find_last_of("/\\");
  return in_path.substr(0, found);
}
} // namespace path
} // namespace utils
