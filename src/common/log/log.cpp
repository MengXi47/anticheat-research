#include "common/log/log.h"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <deque>
#include <mutex>
#include <string>

namespace mxlog {

namespace {

constexpr std::size_t kMaxEntries = 500;

std::mutex g_mu;
std::deque<Entry> g_entries;
std::FILE *g_file = nullptr;
std::string g_file_path;
std::chrono::steady_clock::time_point g_start_time{};

int64_t MillisSinceStart() {
  auto now = std::chrono::steady_clock::now();
  if (g_start_time.time_since_epoch().count() == 0) {
    g_start_time = now;
    return 0;
  }
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - g_start_time).count();
}

const char *LevelTag(Level lv) {
  switch (lv) {
    case Level::Info:
      return "INFO";
    case Level::Warn:
      return "WARN";
    case Level::Error:
      return "ERR ";
  }
  return "????";
}

}  // namespace

void Push(Level level, const char *fmt, ...) {
  char buf[512];
  va_list ap;
  va_start(ap, fmt);
  std::vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  Entry e;
  e.level = level;
  e.text = buf;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    e.ms_since_start = MillisSinceStart();
    g_entries.push_back(e);
    while (g_entries.size() > kMaxEntries) {
      g_entries.pop_front();
    }
    if (g_file) {
      std::fprintf(g_file,
                   "[%8lld] %s  %s\n",
                   static_cast<long long>(e.ms_since_start),
                   LevelTag(e.level),
                   e.text.c_str());
      std::fflush(g_file);
    }
  }
}

std::vector<Entry> Snapshot() {
  std::lock_guard<std::mutex> lk(g_mu);
  return std::vector<Entry>(g_entries.begin(), g_entries.end());
}

std::string SnapshotAsText() {
  std::vector<Entry> v;
  {
    std::lock_guard<std::mutex> lk(g_mu);
    v.assign(g_entries.begin(), g_entries.end());
  }
  std::string out;
  out.reserve(v.size() * 80);
  char prefix[32];
  for (auto &e : v) {
    std::snprintf(prefix, sizeof(prefix), "[%8lld] %s  ", static_cast<long long>(e.ms_since_start), LevelTag(e.level));
    out += prefix;
    out += e.text;
    out += '\n';
  }
  return out;
}

void Clear() {
  std::lock_guard<std::mutex> lk(g_mu);
  g_entries.clear();
}

std::size_t Size() {
  std::lock_guard<std::mutex> lk(g_mu);
  return g_entries.size();
}

void OpenFile(const char *path) {
  std::lock_guard<std::mutex> lk(g_mu);
  if (g_file) {
    std::fclose(g_file);
    g_file = nullptr;
  }
  g_file_path.clear();

  if (!path || !path[0]) return;

  g_file = std::fopen(path, "a");
  if (!g_file) return;
  g_file_path = path;

  std::time_t now_t = std::time(nullptr);
  char ts[64];
  std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S %z", std::gmtime(&now_t));
  std::fprintf(g_file, "\n===== session start ts=%s =====\n", ts);
  std::fflush(g_file);
}

std::string GetFilePath() {
  std::lock_guard<std::mutex> lk(g_mu);
  return g_file_path;
}

}  // namespace mxlog
