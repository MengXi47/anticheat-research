#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace aob {

std::vector<uintptr_t> FindAll(uintptr_t begin, uintptr_t end, const char *pattern, std::size_t max_results = 32);

uintptr_t FindFirst(uintptr_t begin, uintptr_t end, const char *pattern);

uintptr_t DecodeAdrpPlusImm(uintptr_t pc, uint32_t adrp_inst, uint32_t add_inst);

}  // namespace aob
