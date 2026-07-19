#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace mxlog {

enum class Level : uint8_t {
  Info = 0,
  Warn = 1,
  Error = 2,
};

struct Entry {
  int64_t ms_since_start = 0;
  Level level = Level::Info;
  std::string text;
};

void Push(Level level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

std::vector<Entry> Snapshot();
std::string SnapshotAsText();

void Clear();
std::size_t Size();
void OpenFile(const char *path);
std::string GetFilePath();

}  // namespace mxlog

#define MXLOG_INFO(fmt, ...) ::mxlog::Push(::mxlog::Level::Info, fmt, ##__VA_ARGS__)
#define MXLOG_WARN(fmt, ...) ::mxlog::Push(::mxlog::Level::Warn, fmt, ##__VA_ARGS__)
#define MXLOG_ERROR(fmt, ...) ::mxlog::Push(::mxlog::Level::Error, fmt, ##__VA_ARGS__)

#define MXLOG_INFO_ONCE(fmt, ...)                              \
  do {                                                         \
    static bool s_mxlog_once_info_logged = false;              \
    if (!s_mxlog_once_info_logged) {                           \
      ::mxlog::Push(::mxlog::Level::Info, fmt, ##__VA_ARGS__); \
      s_mxlog_once_info_logged = true;                         \
    }                                                          \
  } while (0)

#define MXLOG_WARN_ONCE(fmt, ...)                              \
  do {                                                         \
    static bool s_mxlog_once_warn_logged = false;              \
    if (!s_mxlog_once_warn_logged) {                           \
      ::mxlog::Push(::mxlog::Level::Warn, fmt, ##__VA_ARGS__); \
      s_mxlog_once_warn_logged = true;                         \
    }                                                          \
  } while (0)

#define MXLOG_ERROR_ONCE(fmt, ...)                              \
  do {                                                          \
    static bool s_mxlog_once_err_logged = false;                \
    if (!s_mxlog_once_err_logged) {                             \
      ::mxlog::Push(::mxlog::Level::Error, fmt, ##__VA_ARGS__); \
      s_mxlog_once_err_logged = true;                           \
    }                                                           \
  } while (0)
