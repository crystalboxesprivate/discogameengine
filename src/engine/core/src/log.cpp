#include <core/log.h>
#include <utils/path.h>

using namespace logging;

Logger *logging::g_logger = nullptr;

Logger::Logger() : filename("log.txt") { init(); }

Logger::Logger(const String& in_filename) : filename(in_filename) { init(); }

void Logger::init() {
  if (g_logger) {
    // Logger already exists.
    return;
  }

  {
    // check if path exists
    // read the file to string
  }

  // Clean the old log file.
  {
    FILE *file;
    file = fopen(filename.c_str(), "w");
    const char *header = "";
    fwrite(header, 1, strlen(header), file);
    fclose(file);
  }

  logging::g_logger = this;
  is_valid = true;
}

Logger::~Logger() {
  if (log_file) {
    fclose(log_file);
    log_file = nullptr;
  }
  if (!is_valid) {
    return;
  }
  logging::g_logger = nullptr;
}

const char *logging::get_log_type_string(LogType::Type log_type) {
  switch (log_type) {
  case LogType::System:
    return "System";
  case LogType::Shaders:
    return "Shaders";
  case LogType::Temp:
    return "Temp";
  case LogType::Rendering:
    return "Rendering";
  case LogType::Physics:
    return "Physics";
  case LogType::Assets:
    return "Assets";
  case LogType::Game:
    return "Game";
  default:
    return "Default";
  };
}
