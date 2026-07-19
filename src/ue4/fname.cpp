#include "ue4/fname.h"

#include <algorithm>
#include <cstddef>
#include <cstring>

#include "SDK/SDK.hpp"
#include "common/log/log.h"
#include "common/ptr/valid.h"

namespace ue4 {

namespace {

constexpr std::size_t kElementsPerChunk = 16384;
constexpr std::size_t kChunkTableSize = 128;

}  // namespace

int32_t FindFNameIndex(const char *needle) {
  const auto pool_ptr = SDK::FName::GNames;
  if (!pool_ptr || !needle) {
    return 0;
  }
  const auto pool_addr = reinterpret_cast<uintptr_t>(pool_ptr);

  const int32_t num_elements = *reinterpret_cast<const int32_t *>(pool_addr + kChunkTableSize * sizeof(void *));
  if (num_elements <= 0 || num_elements > static_cast<int32_t>(2 * 1024 * 1024)) {
    MXLOG_ERROR_ONCE("ue4::FindFNameIndex: GNames NumElements out of range (%d)", num_elements);
    return 0;
  }

  const auto chunks_base = reinterpret_cast<SDK::FNameEntry *const *const *>(pool_addr);

  int null_chunks = 0;
  for (std::size_t ci = 0; ci < kChunkTableSize; ++ci) {
    auto *chunk = chunks_base[ci];
    const std::size_t chunk_first = ci * kElementsPerChunk;
    if (chunk_first >= static_cast<std::size_t>(num_elements)) {
      break;
    }
    if (!chunk) {
      ++null_chunks;
      continue;
    }
    if (!ptr::IsValid(reinterpret_cast<uintptr_t>(chunk))) {
      MXLOG_WARN_ONCE("ue4::FindFNameIndex: chunk[%zu] = %p 非 user-space,skip", ci, static_cast<const void *>(chunk));
      continue;
    }

    const std::size_t chunk_end = std::min(chunk_first + kElementsPerChunk, static_cast<std::size_t>(num_elements));

    for (std::size_t ei = chunk_first; ei < chunk_end; ++ei) {
      auto *entry = chunk[ei - chunk_first];
      if (!entry) continue;
      if (!ptr::IsValid(reinterpret_cast<uintptr_t>(entry))) continue;
      if (entry->IsWide()) continue;
      const char *name = entry->GetAnsiName();
      if (!name) continue;
      if (std::strcmp(name, needle) == 0) {
        return static_cast<int32_t>(ei);
      }
    }
  }
  MXLOG_WARN_ONCE("ue4::FindFNameIndex: '%s' not found (NumElements=%d, null_chunks=%d)",
                  needle,
                  num_elements,
                  null_chunks);
  return 0;
}

}  // namespace ue4
