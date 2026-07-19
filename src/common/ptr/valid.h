#pragma once

#include <cstdint>

namespace ptr {

inline constexpr uintptr_t kMinUserPtr = 0x100000000ULL;
inline constexpr uintptr_t kMaxUserPtr = 0x800000000ULL;

inline bool IsValid(uintptr_t p) { return p >= kMinUserPtr && p < kMaxUserPtr; }

inline bool IsValid(const void *p) { return IsValid(reinterpret_cast<uintptr_t>(p)); }

}  // namespace ptr
