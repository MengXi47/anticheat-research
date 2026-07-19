#include "state/esp_buffer.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

namespace state::esp_buffer {

namespace {

using FramePtr = std::shared_ptr<const std::vector<Player>>;

FramePtr g_latest = std::make_shared<const std::vector<Player>>();

std::atomic<std::int64_t> g_last_publish_ns{0};

std::int64_t NowNs() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

}  // namespace

void Publish(std::vector<Player> frame) {
  auto next = std::make_shared<const std::vector<Player>>(std::move(frame));
  std::atomic_store_explicit(&g_latest, std::move(next), std::memory_order_release);
  g_last_publish_ns.store(NowNs(), std::memory_order_release);
}

FramePtr Snapshot() { return std::atomic_load_explicit(&g_latest, std::memory_order_acquire); }

bool IsFresh(int max_age_ms) {
  const std::int64_t last = g_last_publish_ns.load(std::memory_order_acquire);
  if (last == 0) {
    return false;
  }
  return (NowNs() - last) <= static_cast<std::int64_t>(max_age_ms) * 1'000'000;
}

}  // namespace state::esp_buffer
