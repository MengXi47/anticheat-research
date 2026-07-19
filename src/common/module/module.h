#pragma once

#include <cstddef>
#include <cstdint>

namespace mach {

struct Module {
  const char *name = nullptr;
  uintptr_t base = 0;
  uintptr_t text_ = 0;
  uintptr_t text_end = 0;
  std::size_t image_size = 0;

  bool IsValid() const { return base != 0 && text_end > text_; }
};

bool FindModuleByName(const char *basename_or_needle, Module &out);

}  // namespace mach
