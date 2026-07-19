#pragma once

#include <cstddef>
#include <cstdint>

namespace mem {

bool WriteRaw(uintptr_t dst, const void *src, std::size_t size);

template <typename T>
inline bool Write(uintptr_t dst, const T &value) {
  return WriteRaw(dst, &value, sizeof(T));
}

}  // namespace mem
