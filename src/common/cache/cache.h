#pragma once

#include <atomic>
#include <cstdint>

#include "common/module/module.h"

namespace cache {

inline std::atomic<mach::Module> Module{};
inline std::atomic<uintptr_t> GetGNamesFn{0};
inline std::atomic<uintptr_t> DecryptActorArrayFn{0};
inline std::atomic<uintptr_t> GUObjectArraySlot{0};
inline std::atomic<uintptr_t> GUObjectArray{0};
inline std::atomic<std::uint32_t> GUObjectArrayNum{0};
inline std::atomic<uintptr_t> GNamesPool{0};

}  // namespace cache
