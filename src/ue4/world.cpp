#include "world.h"

#include "common/cache/cache.h"
#include "common/log/log.h"
#include "common/ptr/valid.h"
#include "ue4/character.h"
#include "ue4/offset.h"

namespace ue4 {

namespace {

SDK::UObject *SafeFindObjectByName(const char *full_name) {
  auto &objs = SDK::UObject::GetGlobalObjects();
  const int num = objs.Num();
  for (int i = 0; i < num; ++i) {
    SDK::UObject *obj = objs.GetByIndex(i);
    if (!ptr::IsValid(obj)) {
      continue;
    }
    if (!ue4::character::IsLikelyValidUObject(obj)) {
      continue;
    }
    if (obj->GetFullName() == full_name) {
      return obj;
    }
  }
  return nullptr;
}

}  // namespace

SDK::UWorld *GetWorld() {
  if (!SDK::UObject::GUObjectArray) {
    MXLOG_ERROR_ONCE("ue4::GetWorld: SDK::UObject::GUObjectArray null");
    return nullptr;
  }

  static SDK::UEngine *engine = nullptr;
  if (!engine) {
    engine = static_cast<SDK::UEngine *>(SafeFindObjectByName(pubg::kEngineFullName));
    if (!engine) {
      MXLOG_WARN_ONCE("ue4::GetWorld: FindObject('%s') not found engine object", pubg::kEngineFullName);
      return nullptr;
    }
    MXLOG_INFO("ue4::GetWorld: cached engine = %p", static_cast<void *>(engine));
  }

  SDK::UGameViewportClient *viewport = engine->GameViewport;
  if (!ptr::IsValid(viewport)) {
    MXLOG_ERROR_ONCE("ue4::GetWorld: engine->GameViewport Valid (%p)", static_cast<void *>(viewport));
    return nullptr;
  }
  return viewport->World;
}

SDK::TArray<SDK::AActor *> GetActors(SDK::ULevel *level) {
  if (!ptr::IsValid(level)) {
    MXLOG_ERROR_ONCE("ue4::GetActors: level invalid (%p)", static_cast<void *>(level));
    return {};
  }

  const uintptr_t fn = cache::DecryptActorArrayFn.load();
  if (!ptr::IsValid(fn)) {
    MXLOG_ERROR_ONCE("ue4::GetActors: DecryptActorArrayFn undefined (val=0x%lx)", fn);
    return {};
  }

  using DecryptFn = SDK::TArray<SDK::AActor *> *(*)(SDK::ULevel *);
  auto *arr_ptr = reinterpret_cast<DecryptFn>(fn)(level);
  if (!ptr::IsValid(arr_ptr)) {
    MXLOG_WARN_ONCE("ue4::GetActors: DecryptFn return invalid ptr (%p)", static_cast<void *>(arr_ptr));
    return {};
  }
  return *arr_ptr;
}

SDK::APlayerController *GetLocalPlayerController(SDK::UWorld *world) {
  if (!ptr::IsValid(world)) {
    MXLOG_ERROR_ONCE("ue4::GetLocalPlayerController: world invalid (%p)", static_cast<void *>(world));
    return nullptr;
  }
  const auto wbase = reinterpret_cast<uintptr_t>(world);
  const auto net = *reinterpret_cast<uintptr_t *>(wbase + offset::kNetDriver);
  if (!ptr::IsValid(net)) {
    MXLOG_WARN_ONCE("ue4::GetLocalPlayerController: NetDriver null (0x%lx)", net);
    return nullptr;
  }
  const auto conn = *reinterpret_cast<uintptr_t *>(net + offset::kServerConnection);
  if (!ptr::IsValid(conn)) {
    MXLOG_WARN_ONCE("ue4::GetLocalPlayerController: ServerConnection null (0x%lx)", conn);
    return nullptr;
  }
  return *reinterpret_cast<SDK::APlayerController **>(conn + offset::kLocalPlayerController);
}

SDK::APawn *GetLocalPawn(SDK::APlayerController *controller) {
  if (!ptr::IsValid(controller)) {
    MXLOG_WARN_ONCE("ue4::GetLocalPawn: controller 尚未 ready (%p)", static_cast<void *>(controller));
    return nullptr;
  }
  return *reinterpret_cast<SDK::APawn **>(reinterpret_cast<uintptr_t>(controller) + pubg_offset::kSTBaseCharacter);
}

std::optional<SDK::FVector> GetActorLocation(SDK::AActor *actor) {
  if (!ptr::IsValid(actor)) {
    return std::nullopt;
  }
  const auto root = *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(actor) + offset::kRootComponent);
  if (!ptr::IsValid(root)) {
    return std::nullopt;
  }
  return *reinterpret_cast<SDK::FVector *>(root + offset::kRelativeLocation);
}

}  // namespace ue4
