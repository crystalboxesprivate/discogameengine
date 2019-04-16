#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <core/typedefs.h>

#define DEBUG 1
#if DEBUG
#define DEBUG_LOG(LoggingType, Verbosity, Format, ...)                                             \
  logging::Logger::log<logging::LogType::LoggingType, logging::VerbosityType::Verbosity>(          \
      __FILE__, __LINE__, Format, __VA_ARGS__);
#else
#define DEBUGLOG(LogType, Format, ...)
#endif

namespace logging {
namespace LogType {
enum Type { System, Temp, Rendering, Physics, Assets, Game, Shaders };
}
namespace VerbosityType {
enum Type { Log, Warning, Error };
}

class Logger;

extern Logger *g_logger;
const char *get_log_type_string(LogType::Type log_type);

class Logger {
public:
  Logger();
  Logger(const String& in_filename);
  ~Logger();
  // Log function main.
  template <LogType::Type logging_type, VerbosityType::Type verbosity_type>
  static void log(const char *file, int line, const char *fmt, ...) {
    static const usize MAX_BUF_SIZE = 512;
    // char out_buf[MAX_BUF_SIZE];
    {
      const char *verbosity = "";
      {
        // convert enum to string
        switch (verbosity_type) {
        case VerbosityType::Error:
          verbosity = "Error";
          break;
        case VerbosityType::Warning:
          verbosity = "Warning";
          break;
        case VerbosityType::Log:
        default:
          verbosity = "Log";
          break;
        }
      }

      const char *logging_type_string = get_log_type_string(logging_type);
      // Write processed vargs into the buffer.
      // char formatted[MAX_BUF_SIZE];
      {
        va_list va;
        va_start(va, fmt);
        vsprintf(g_logger->formatted, fmt, va);
        va_end(va);
      }
      // Get nicely formatted time.
      char time_buf[36];
      {
        time_t current_time = time(NULL);
        struct tm *ptm = gmtime(&current_time);
        strftime(time_buf, sizeof(time_buf), "%T", ptm);
      }

      if (verbosity_type != VerbosityType::Log) {
        // For errors and warnings show line and the filename where the error did happen.
        // Get filename from __FILE_ is taken from here https://stackoverflow.com/a/8488201
        const char *source_filename = strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file;
        sprintf(g_logger->out_buf, "[%s] [%s] %s (%s:%d): %s\n", time_buf, verbosity, logging_type_string,
                source_filename, line, g_logger->formatted);
      } else
        sprintf(g_logger->out_buf, "[%s] %s: %s\n", time_buf, logging_type_string, g_logger->formatted);
    }
    // Console output.
    printf(g_logger->out_buf);

    if (g_logger)
      g_logger->write_to_file(g_logger->out_buf);
  }

  void write_to_file(const char *out_buf) {
    if (!is_valid)
      return;

    #if 0
    if (log_file == 0) {
      log_file = fopen(filename.c_str(), "at");
      if (!log_file)
        log_file = fopen(filename.c_str(), "wt");
      if (!log_file) {
        printf("can not open %s for writing.\n", filename.c_str());
        return;
      }
    }
    fwrite(out_buf, 1, strlen(out_buf), log_file);
    #else
      log_file = fopen(filename.c_str(), "at");
      fwrite(out_buf, 1, strlen(out_buf), log_file);
      fclose(log_file);
      log_file = nullptr;
    #endif
  }

private:
  // Constructor
  void init();

  char out_buf[4096];
  char formatted[4096];

  String filename = nullptr;
  bool is_valid = false;
  FILE *log_file = 0;
};
} // namespace logging
