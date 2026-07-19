#pragma once

#include <cstdint>

namespace hook {

enum class SwapResult : std::uint8_t {
  kInstalled,
  kAlreadyHooked,
  kFailed,
};

SwapResult SwapVTableSlot(void *instance, std::uint32_t slot, void *replacement, void **out_original);

}  // namespace hook
