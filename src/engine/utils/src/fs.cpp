#include <utils/fs.h>
#include <utils/path.h>
#include <engine>
#include <fstream>
#include <sys/stat.h>
#include <direct.h>

namespace utils {
namespace fs {
bool create_directory(const String &name) {
  return _mkdir(name.c_str()) == 0;
}
bool load_file_to_string(const String &filename, String &contents) {
  using namespace std;
  ifstream input_stream(filename);
  if (input_stream) {
    input_stream.seekg(0, ios::end);
    size_t char_count = input_stream.tellg();
    input_stream.seekg(0);
    contents = String(char_count + 1, '\0');
    input_stream.read(&contents[0], char_count);
    return true;
  }
  DEBUG_LOG(System, Error, "Couldn't open %s.", filename.c_str());
  return false;
}

bool load_file_to_buffer(const String &filename, Vector<u8> &data) {
  struct stat results;
  if (stat(filename.c_str(), &results) == 0) {
    data.reserve(results.st_size);
    FILE *file;
    file = fopen(filename.c_str(), "rb");
    fread(data.data(), 1, results.st_size, file);
    return true;
  } else
    DEBUG_LOG(System, Error, "Couldn't open %s.", filename.c_str());
  return false;
}

bool save_binary_file(const String &filename, usize size, void *data) {
  assert(size);
  assert(data);
  FILE *write_ptr;
  write_ptr = fopen(filename.c_str(), "wb");
  assert(write_ptr);
  fwrite(data, 1, size, write_ptr);
  fclose(write_ptr);
  return true;
}
} // namespace fs
} // namespace utils
