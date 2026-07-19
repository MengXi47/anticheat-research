#include "character.h"

#include <cstdint>

#include "common/log/log.h"
#include "common/ptr/valid.h"

namespace ue4::character {

bool IsLikelyValidUObject(SDK::UObject *obj) {
  const auto base = reinterpret_cast<uintptr_t>(obj);

  const auto vtable = *reinterpret_cast<uintptr_t *>(base);
  if (!ptr::IsValid(vtable)) {
    MXLOG_WARN_ONCE("IsLikelyValidUObject: reject - bad VTable @0x00 (obj=%p vtable=0x%lx)",
                    static_cast<void *>(obj),
                    vtable);
    return false;
  }

  const int32_t internal_idx = *reinterpret_cast<int32_t *>(base + 0x0C);
  if (internal_idx < 0 || internal_idx >= 2'000'000) {
    MXLOG_WARN_ONCE(
        "IsLikelyValidUObject: reject - InternalIndex @0x0C out of range "
        "(obj=%p idx=%d)",
        static_cast<void *>(obj),
        internal_idx);
    return false;
  }

  const auto class_ptr = *reinterpret_cast<uintptr_t *>(base + 0x10);
  if (!ptr::IsValid(class_ptr)) {
    MXLOG_WARN_ONCE(
        "IsLikelyValidUObject: reject - bad ClassPrivate @0x10 "
        "(obj=%p class=0x%lx)",
        static_cast<void *>(obj),
        class_ptr);
    return false;
  }

  const auto class_vtable = *reinterpret_cast<uintptr_t *>(class_ptr);
  if (!ptr::IsValid(class_vtable)) {
    MXLOG_WARN_ONCE(
        "IsLikelyValidUObject: reject - ClassPrivate has bad vtable "
        "(obj=%p class=0x%lx classVtable=0x%lx)",
        static_cast<void *>(obj),
        class_ptr,
        class_vtable);
    return false;
  }

  return true;
}

bool IsPlayerCharacter(SDK::UObject *obj) {
  if (!ptr::IsValid(obj)) {
    return false;
  }
  if (!SDK::UObject::GUObjectArray || !SDK::FName::GNames) {
    MXLOG_ERROR_ONCE("character::IsPlayerCharacter: SDK not ready (GUObjectArray=%p GNames=%p)",
                     static_cast<void *>(SDK::UObject::GUObjectArray),
                     static_cast<void *>(SDK::FName::GNames));
    return false;
  }

  if (!IsLikelyValidUObject(obj)) {
    return false;
  }

  static SDK::UClass *player_class = nullptr;
  if (!player_class) {
    player_class = SDK::ASTExtraPlayerCharacter::StaticClass();
    if (!player_class) {
      MXLOG_ERROR_ONCE(
          "character::IsPlayerCharacter: ASTExtraPlayerCharacter::StaticClass() "
          "returned null");
      return false;
    }
    MXLOG_INFO("character: cached ASTExtraPlayerCharacter::StaticClass = %p", static_cast<void *>(player_class));
  }

  return obj->IsA(player_class);
}

}  // namespace ue4::character
