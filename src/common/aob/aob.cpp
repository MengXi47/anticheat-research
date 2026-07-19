#include "common/aob/aob.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

namespace aob {

namespace {

bool ParsePattern(const char *pattern, std::vector<uint8_t> &bytes, std::vector<uint8_t> &mask) {
  bytes.clear();
  mask.clear();

  const char *p = pattern;
  while (*p) {
    while (*p == ' ' || *p == '\t') ++p;
    if (!*p) break;

    if (*p == '?') {
      bytes.push_back(0);
      mask.push_back(0);
      ++p;
      if (*p == '?') ++p;
      continue;
    }

    if (!std::isxdigit(static_cast<unsigned char>(p[0])) || !std::isxdigit(static_cast<unsigned char>(p[1]))) {
      return false;
    }
    char buf[3] = {p[0], p[1], 0};
    bytes.push_back(static_cast<uint8_t>(std::strtoul(buf, nullptr, 16)));
    mask.push_back(0xFF);
    p += 2;
  }
  return !bytes.empty();
}

}  // namespace

std::vector<uintptr_t> FindAll(uintptr_t begin, uintptr_t end, const char *pattern, std::size_t max_results) {
  std::vector<uintptr_t> hits;
  if (begin == 0 || end <= begin) return hits;

  std::vector<uint8_t> bytes, mask;
  if (!ParsePattern(pattern, bytes, mask)) return hits;

  const std::size_t pat_len = bytes.size();
  const std::size_t scan_size = end - begin;
  if (scan_size < pat_len) return hits;

  const auto *haystack = reinterpret_cast<const uint8_t *>(begin);
  for (std::size_t i = 0; i + pat_len <= scan_size; ++i) {
    bool match = true;
    for (std::size_t j = 0; j < pat_len; ++j) {
      if (mask[j] && haystack[i + j] != bytes[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      hits.push_back(reinterpret_cast<uintptr_t>(haystack + i));
      if (hits.size() >= max_results) break;
    }
  }
  return hits;
}

uintptr_t FindFirst(uintptr_t begin, uintptr_t end, const char *pattern) {
  auto v = FindAll(begin, end, pattern, 1);
  return v.empty() ? 0 : v[0];
}

uintptr_t DecodeAdrpPlusImm(uintptr_t pc, uint32_t adrp, uint32_t add) {
  int64_t immlo = (adrp >> 29) & 0x3;
  int64_t immhi = (adrp >> 5) & 0x7FFFF;
  int64_t imm21 = (immhi << 2) | immlo;

  if (imm21 & (1LL << 20)) {
    imm21 |= ~((1LL << 21) - 1);
  }
  int64_t page_offset = imm21 << 12;

  uintptr_t base_page = pc & ~uintptr_t{0xFFF};
  uintptr_t adrp_target = static_cast<uintptr_t>(static_cast<int64_t>(base_page) + page_offset);

  uint32_t opcode_top = (add >> 24) & 0xFF;
  uint64_t imm12 = (add >> 10) & 0xFFF;

  if (opcode_top == 0xF9) {
    return adrp_target + (imm12 << 3);
  }
  return adrp_target + imm12;
}

}  // namespace aob
