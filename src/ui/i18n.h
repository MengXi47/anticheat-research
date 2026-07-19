#pragma once

#include <atomic>

namespace ui::i18n {

enum class Lang { Zh, En };

inline std::atomic<Lang> g_lang{Lang::Zh};

inline Lang Current() {
  return g_lang.load(std::memory_order_relaxed);
}
inline void Set(Lang l) {
  g_lang.store(l, std::memory_order_relaxed);
}

inline const char* T(const char* zh, const char* en) {
  return Current() == Lang::En ? en : zh;
}

} // namespace ui::i18n
