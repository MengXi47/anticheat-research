#pragma once

#include <atomic>
#include <thread>

namespace runtimebooter {

class RuntimeBooter {
 public:
  static RuntimeBooter &Instance();

  RuntimeBooter(const RuntimeBooter &) = delete;
  RuntimeBooter &operator=(const RuntimeBooter &) = delete;

  void Start();
  void Shutdown();

  bool IsDone() const { return done_.load(std::memory_order_acquire); }

 private:
  RuntimeBooter() = default;
  ~RuntimeBooter() { Shutdown(); }

  void Run(const std::stop_token &sn);

  std::atomic<bool> started_{false};
  std::atomic<bool> done_{false};
  std::jthread AOB_Thread_;
};
}  // namespace runtimebooter