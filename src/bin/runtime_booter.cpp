#include "runtime_booter.h"

#include "SDK/SDK.hpp"
#include "common/aob/aob.h"
#include "common/cache/cache.h"
#include "common/log/log.h"
#include "common/module/module.h"
#include "common/ptr/valid.h"
#include "hook/process_event.h"
#include "ue4/offset.h"

namespace runtimebooter {

constexpr auto kModuleName = "ShadowTrackerExtra";

RuntimeBooter &RuntimeBooter::Instance() {
  static RuntimeBooter instance;
  return instance;
}

void RuntimeBooter::Start() {
  if (bool expected = false; !started_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
    return;
  }
  AOB_Thread_ = std::jthread([this](const std::stop_token &st) { Run(st); });
}

void RuntimeBooter::Shutdown() {
  if (AOB_Thread_.joinable()) {
    AOB_Thread_.request_stop();
    AOB_Thread_.join();
  }
}

void RuntimeBooter::Run(const std::stop_token &sn) {
  mach::Module mod{};
  while (!sn.stop_requested()) {
    mach::FindModuleByName(kModuleName, mod);
    if (mod.IsValid()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  if (!mod.IsValid()) {
    done_.store(true, std::memory_order_release);
    return;
  }
  cache::Module.store(mod);
  MXLOG_INFO("Module %s found", kModuleName);

  const uintptr_t GetGNameFn = aob::FindFirst(mod.text_, mod.text_end, ue4::pattern::kGetGNamesFn);
  if (GetGNameFn != 0) {
    cache::GetGNamesFn.store(GetGNameFn);
    MXLOG_INFO("GetGNames AOB hit: 0x%lx", GetGNameFn);
  } else {
    MXLOG_ERROR("GetGNames AOB pattern not found");
  }

  const uintptr_t DecryptActorArrayFn = aob::FindFirst(mod.text_, mod.text_end, ue4::pattern::kDecryptActorArrayFn);
  if (DecryptActorArrayFn != 0) {
    cache::DecryptActorArrayFn.store(DecryptActorArrayFn);
    MXLOG_INFO("DecryptActorArray AOB hit: 0x%lx", DecryptActorArrayFn);
  } else {
    MXLOG_ERROR("DecryptActorArray AOB pattern not found");
  }

  const uintptr_t GUObjectArrayHit = aob::FindFirst(mod.text_, mod.text_end, ue4::pattern::kGUObjectArray);
  if (GUObjectArrayHit == 0) {
    MXLOG_ERROR("GUObjectArray AOB pattern not found");
    done_.store(true, std::memory_order_release);
    return;
  }
  const uintptr_t adrp_pc = GUObjectArrayHit + 2;
  const uintptr_t GUObjectArraySlot = aob::DecodeAdrpPlusImm(adrp_pc,
                                                             *reinterpret_cast<const uint32_t *>(adrp_pc),
                                                             *reinterpret_cast<const uint32_t *>(adrp_pc + 4));
  cache::GUObjectArraySlot.store(GUObjectArraySlot);
  MXLOG_INFO("GUObjectArray slot: 0x%lx", GUObjectArraySlot);

  constexpr uint32_t kMinReasonableN = 100;
  constexpr uint32_t kMaxReasonableN = 10'000'000;

  uint32_t num_elements = 0;
  while (!sn.stop_requested()) {
    const auto n = *reinterpret_cast<const uint32_t *>(GUObjectArraySlot + ue4::offset::kNumElements);
    const auto objects = *reinterpret_cast<const uintptr_t *>(GUObjectArraySlot + ue4::offset::kObjectsPtr);
    if (n >= kMinReasonableN && n <= kMaxReasonableN && ptr::IsValid(objects)) {
      num_elements = n;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  if (num_elements != 0) {
    cache::GUObjectArray.store(GUObjectArraySlot);
    cache::GUObjectArrayNum.store(num_elements);
    MXLOG_INFO("GUObjectArray ready: 0x%lx (Num=%u)", GUObjectArraySlot, num_elements);

    SDK::UObject::GUObjectArray = reinterpret_cast<SDK::FUObjectArray *>(GUObjectArraySlot);
    MXLOG_INFO("SDK::UObject::GUObjectArray assigned");
  }

  if (GetGNameFn != 0) {
    if (GetGNameFn < mod.text_ || GetGNameFn >= mod.text_end) {
      MXLOG_ERROR("GetGNameFn 0x%lx out of __text range, abort", GetGNameFn);
    } else {
      using GetGNamesFnT = SDK::TNameEntryArray *(*)();
      auto *pool = reinterpret_cast<GetGNamesFnT>(GetGNameFn)();
      const auto pu = reinterpret_cast<uintptr_t>(pool);
      const bool ok = pool != nullptr && ptr::IsValid(pu) && [&] {
        const auto c = reinterpret_cast<uintptr_t>(*reinterpret_cast<void *const *>(pool));
        return ptr::IsValid(c);
      }();

      if (ok) {
        cache::GNamesPool.store(pu);
        SDK::FName::GNames = pool;
        MXLOG_INFO("SDK::FName::GNames assigned: %p", static_cast<void *>(pool));
      } else {
        MXLOG_ERROR("GNames by-call 回傳 %p 未通過 sanity check", static_cast<void *>(pool));
      }
    }
  }

  done_.store(true, std::memory_order_release);

  while (!sn.stop_requested()) {
    if (pehook::TryInstall()) {
      MXLOG_INFO("pehook: ProcessEvent hook installed");
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

}  // namespace runtimebooter
