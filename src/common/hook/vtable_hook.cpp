#include "common/hook/vtable_hook.h"

#include <cstdint>

#include "common/log/log.h"
#include "common/mem/mem.h"
#include "common/ptr/valid.h"

namespace hook {

SwapResult SwapVTableSlot(void *instance, std::uint32_t slot, void *replacement, void **out_original) {
  if (!ptr::IsValid(instance) || replacement == nullptr) {
    MXLOG_ERROR("SwapVTableSlot bad args instance=%p replacement=%p", instance, replacement);
    return SwapResult::kFailed;
  }
  auto **vtable = *reinterpret_cast<void ***>(instance);
  if (!ptr::IsValid(reinterpret_cast<uintptr_t>(vtable))) {
    MXLOG_ERROR("SwapVTableSlot bad vtable instance=%p vtable=%p", instance, vtable);
    return SwapResult::kFailed;
  }
  void *current = vtable[slot];
  if (!ptr::IsValid(reinterpret_cast<uintptr_t>(current))) {
    MXLOG_ERROR("SwapVTableSlot bad slot vtable=%p slot=%u current=%p", vtable, slot, current);
    return SwapResult::kFailed;
  }
  if (current == replacement) return SwapResult::kAlreadyHooked;

  if (out_original != nullptr) *out_original = current;

  const auto slot_addr = reinterpret_cast<uintptr_t>(&vtable[slot]);
  if (!mem::Write(slot_addr, replacement)) {
    MXLOG_ERROR("SwapVTableSlot write fail slot_addr=0x%lx slot=%u", slot_addr, slot);
    return SwapResult::kFailed;
  }
  MXLOG_INFO("SwapVTableSlot ok vtable=%p slot=%u %p -> %p", vtable, slot, current, replacement);
  return SwapResult::kInstalled;
}

}  // namespace hook
