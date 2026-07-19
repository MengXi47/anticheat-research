#include "hook/process_event.h"

#include <atomic>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <vector>

#include "SDK/SDK.hpp"
#include "common/hook/vtable_hook.h"
#include "common/log/log.h"
#include "common/ptr/valid.h"
#include "imgui.h"
#include "state/aim_state.h"
#include "state/esp_buffer.h"
#include "state/esp_state.h"
#include "state/item_buffer.h"
#include "ue4/aim.h"
#include "ue4/character.h"
#include "ue4/fname.h"
#include "ue4/items_table.h"
#include "ue4/offset.h"
#include "ue4/player_data.h"
#include "ue4/projection.h"
#include "ue4/skeleton.h"
#include "ue4/touch_aim.h"
#include "ue4/world.h"
#include "ui/i18n.h"

namespace pehook {

namespace {

constexpr std::uint32_t kProcessEventSlot = ue4::offset::kVTableSlotProcessEvent / sizeof(void *);

constexpr const char *kTargetClasses[] = {
    "Class ShadowTrackerExtra.SurviveHUD",
};
constexpr int kTargetCount = std::size(kTargetClasses);

using ProcessEventFn = void (*)(void *self, void *function, void *params);

std::atomic<ProcessEventFn> g_original{nullptr};
std::atomic<bool> g_installed{false};

constexpr ImU32 kColorCanHit = IM_COL32(0, 255, 0, 230);
constexpr ImU32 kColorBlocked = IM_COL32(255, 80, 80, 200);

bool IsReceiveDrawHUD(void *function) {
  if (function == nullptr) {
    return false;
  }
  static int32_t s_recv_draw_hud_idx = 0;
  if (s_recv_draw_hud_idx == 0) {
    s_recv_draw_hud_idx = ue4::FindFNameIndex("ReceiveDrawHUD");
    if (s_recv_draw_hud_idx == 0) {
      return false;
    }
  }
  auto *fn = reinterpret_cast<SDK::UObject *>(function);
  return fn->NamePrivate.ComparisonIndex == s_recv_draw_hud_idx;
}

bool IsDeathBox(SDK::AActor *actor) {
  static SDK::UClass *cls = nullptr;
  if (cls == nullptr) {
    cls = SDK::APlayerTombBox::StaticClass();
  }
  return cls != nullptr && actor->IsA(cls);
}

bool IsPickUp(SDK::AActor *actor) {
  static SDK::UClass *cls = nullptr;
  if (cls == nullptr) {
    cls = SDK::APickUpWrapperActor::StaticClass();
  }
  return cls != nullptr && actor->IsA(cls);
}

bool IsVehicle(SDK::AActor *actor) {
  static SDK::UClass *cls = nullptr;
  if (cls == nullptr) {
    cls = SDK::ASTExtraVehicleBase::StaticClass();
  }
  return cls != nullptr && actor->IsA(cls);
}

int SelectAimBone(const state::Player &p, ue4::skeleton::Bone primary) {
  using ue4::skeleton::Bone;
  const int primary_idx = static_cast<int>(primary);
  if (p.points[primary_idx].visible) {
    return primary_idx;
  }
  static constexpr Bone kFallback[] = {
      Bone::Head,
      Bone::Neck,
      Bone::Spine03,
      Bone::Spine02,
      Bone::Spine01,
      Bone::Pelvis,
      Bone::UpperArmR,
      Bone::UpperArmL,
      Bone::ThighR,
      Bone::ThighL,
      Bone::LowerArmR,
      Bone::LowerArmL,
      Bone::CalfR,
      Bone::CalfL,
      Bone::HandR,
      Bone::HandL,
      Bone::FootR,
      Bone::FootL,
  };
  for (const Bone b : kFallback) {
    const int idx = static_cast<int>(b);
    if (p.points[idx].visible) {
      return idx;
    }
  }
  return -1;
}

struct FrameContext {
  SDK::APlayerController *controller = nullptr;
  SDK::AActor *local_pawn = nullptr;
  SDK::APlayerCameraManager *camera_manager = nullptr;
  SDK::FVector camera_location{};
  bool camera_valid = false;
  SDK::FVector local_pos{};
  const ue4::projection::ViewProjection *view = nullptr;
  int local_team_id = 0;
  int item_lang = 0;
  float max_player_dist_sq = 0.0f;
  float max_item_dist_sq = 0.0f;
};

struct AimParams {
  bool enabled = false;
  float fov_radius = 0.0f;
  float max_distance = 0.0f;
  bool ignore_bot = false;
  bool ignore_knocked = false;
  ue4::skeleton::Bone bone = ue4::skeleton::Bone::Head;
};

struct AimSelection {
  SDK::AActor *target = nullptr;
  SDK::FVector bone_world{};
  float best_screen_dist_sq = FLT_MAX;
  float distance = 0.0f;
  bool found = false;
};

struct PlayerResult {
  bool included = false;
  state::Player player;
  ue4::skeleton::Skeleton skeleton;
  float distance = 0.0f;
};

ue4::skeleton::Bone ResolveAimBone(int index) {
  switch (index) {
    case 1:
      return ue4::skeleton::Bone::Neck;
    case 2:
      return ue4::skeleton::Bone::Spine02;
    default:
      return ue4::skeleton::Bone::Head;
  }
}

void PublishEmpty() {
  state::esp_buffer::Publish({});
  state::item_buffer::Publish({});
}

void EmitItem(std::vector<state::Item> &out,
              const FrameContext &ctx,
              const SDK::FVector &loc,
              int category,
              const char *name,
              int id = 0) {
  const float dx = loc.X - ctx.local_pos.X;
  const float dy = loc.Y - ctx.local_pos.Y;
  const float dz = loc.Z - ctx.local_pos.Z;
  const float dist_sq = dx * dx + dy * dy + dz * dz;
  if (dist_sq > ctx.max_item_dist_sq) {
    return;
  }
  state::Item &item = out.emplace_back();
  item.pos = ue4::projection::Projection(*ctx.view, loc);
  item.onscreen = true;
  item.distance = std::sqrt(dist_sq) / 100.0f;
  item.category = category;
  item.name = name;
  item.id = id;
}

void CollectItemActor(SDK::AActor *actor,
                      const FrameContext &ctx,
                      bool show_deathbox,
                      bool show_loot,
                      bool show_vehicle,
                      std::vector<state::Item> &out) {
  if (show_vehicle && IsVehicle(actor)) {
    const auto loc = ue4::GetActorLocation(actor);
    if (loc) {
      const auto vtype =
          *reinterpret_cast<std::uint8_t *>(reinterpret_cast<uintptr_t>(actor) + ue4::pubg_offset::kVehicleType);
      EmitItem(out, ctx, *loc, state::kCategoryVehicle, ue4::VehicleName(vtype, ctx.item_lang));
    }
    return;
  }
  if (show_deathbox && IsDeathBox(actor)) {
    const auto loc = ue4::GetActorLocation(actor);
    if (loc) {
      EmitItem(out, ctx, *loc, state::kCategoryDeathBox, ui::i18n::T("死亡盒", "Death Box"));
    }
    return;
  }
  if (!show_loot || !IsPickUp(actor)) {
    return;
  }
  const auto pickup = reinterpret_cast<uintptr_t>(actor);
  const int item_id = *reinterpret_cast<int *>(pickup + ue4::pubg_offset::kPickUpTypeSpecificID);
  if (item_id == 0) {
    return;
  }
  const ue4::ItemCategory category = ue4::GetItemCategory(item_id);
  const char *name = ue4::GetItemNameByID(item_id, ctx.item_lang);
  if (name != nullptr) {
    if (category == ue4::ItemCategory::Count ||
        !state::g_loot_cat[static_cast<int>(category)].load(std::memory_order_relaxed)) {
      return;
    }
  } else if (!state::g_esp_show_unknown.load(std::memory_order_relaxed)) {
    return;
  }
  const int cat_out = (category == ue4::ItemCategory::Count) ? state::kCategoryUnknownItem : static_cast<int>(category);
  const auto loc = ue4::GetActorLocation(actor);
  if (loc) {
    EmitItem(out, ctx, *loc, cat_out, name, item_id);
  }
}

PlayerResult BuildPlayer(SDK::ASTExtraPlayerCharacter *character, const FrameContext &ctx) {
  PlayerResult r;

  const int team_id = ue4::player_data::ReadTeamId(character);
  if (ctx.local_team_id >= 0 && team_id == ctx.local_team_id) {
    return r;
  }
  const auto health = ue4::player_data::ReadHealth(character);
  if (health.current <= 0.0f) {
    return r;
  }
  ue4::skeleton::GetSkeleton(character, r.skeleton);
  if (!r.skeleton.valid) {
    return r;
  }

  if (ctx.camera_valid) {
    const SDK::FVector &head_world = r.skeleton.At(ue4::skeleton::Bone::Head);
    const float dx = head_world.X - ctx.camera_location.X;
    const float dy = head_world.Y - ctx.camera_location.Y;
    const float dz = head_world.Z - ctx.camera_location.Z;
    const float dist_sq = dx * dx + dy * dy + dz * dz;
    if (dist_sq > ctx.max_player_dist_sq) {
      return r;
    }
    r.distance = std::sqrt(dist_sq) / 100.0f;
  }

  state::Player &out = r.player;

  bool can_hit = false;
  if (ctx.camera_manager != nullptr) {
    const SDK::FVector &enemy_eye = r.skeleton.At(ue4::skeleton::Bone::Head);
    can_hit = ctx.controller->LineOfSightTo(ctx.local_pawn, enemy_eye, true);
  }
  out.color = can_hit ? kColorCanHit : kColorBlocked;

  if (ctx.view != nullptr) {
    for (int b = 0; b < ue4::skeleton::kBoneCount; ++b) {
      out.points[b].pos = ue4::projection::Projection(*ctx.view, r.skeleton.points[b]);
      out.points[b].onscreen = true;
      if (ctx.camera_manager != nullptr) {
        out.points[b].visible = ctx.controller->LineOfSightTo(ctx.camera_manager, r.skeleton.points[b], false);
      }
    }
  }

  out.health = health.current;
  out.health_max = health.max;
  out.knocked = health.knocked;
  out.distance = r.distance;
  out.is_bot = ue4::player_data::IsBot(character);
  out.name = ue4::player_data::ReadName(character);
  const int weapon_id = ue4::player_data::ReadWeaponId(character);
  out.weapon_name = weapon_id != 0 ? ue4::GetItemNameByID(weapon_id, ctx.item_lang) : nullptr;
  out.team_id = team_id;

  r.included = true;
  return r;
}

void ConsiderAimCandidate(SDK::ASTExtraPlayerCharacter *character,
                          const PlayerResult &r,
                          const FrameContext &ctx,
                          const AimParams &ap,
                          AimSelection &sel) {
  if (ctx.view == nullptr || r.distance > ap.max_distance) {
    return;
  }
  if ((ap.ignore_bot && r.player.is_bot) || (ap.ignore_knocked && r.player.knocked)) {
    return;
  }
  const int aim_idx = SelectAimBone(r.player, ap.bone);
  if (aim_idx < 0) {
    return;
  }
  const SDK::FVector &bone_world = r.skeleton.points[aim_idx];
  const float to_bone_x = bone_world.X - ctx.view->cam_loc.X;
  const float to_bone_y = bone_world.Y - ctx.view->cam_loc.Y;
  const float to_bone_z = bone_world.Z - ctx.view->cam_loc.Z;
  const float forward_dot =
      ctx.view->forward.X * to_bone_x + ctx.view->forward.Y * to_bone_y + ctx.view->forward.Z * to_bone_z;
  if (forward_dot <= 0.0f) {
    return;
  }
  const ImVec2 bone_screen = r.player.points[aim_idx].pos;
  const float sdx = bone_screen.x - ctx.view->cx;
  const float sdy = bone_screen.y - ctx.view->cy;
  const float screen_dist_sq = sdx * sdx + sdy * sdy;
  if (screen_dist_sq > ap.fov_radius * ap.fov_radius || screen_dist_sq >= sel.best_screen_dist_sq) {
    return;
  }
  sel.best_screen_dist_sq = screen_dist_sq;
  sel.target = static_cast<SDK::AActor *>(character);
  sel.bone_world = bone_world;
  sel.distance = r.distance;
  sel.found = true;
}

void OnReceiveDrawHUD() {
  auto *world = ue4::GetWorld();
  if (!ptr::IsValid(world)) {
    PublishEmpty();
    return;
  }
  auto *controller = ue4::GetLocalPlayerController(world);
  auto *local_pawn = ue4::GetLocalPawn(controller);
  if (!ptr::IsValid(controller) || !ptr::IsValid(local_pawn)) {
    PublishEmpty();
    return;
  }

  ue4::aim::ApplyNoRecoil(static_cast<SDK::AActor *>(local_pawn));

  auto *level = world->PersistentLevel;
  if (!ptr::IsValid(level)) {
    PublishEmpty();
    return;
  }
  auto actors = ue4::GetActors(level);
  if (actors.Num() <= 0) {
    PublishEmpty();
    return;
  }

  const auto view = ue4::projection::BuildViewProjection(controller);
  SDK::APlayerCameraManager *camera_manager = controller->PlayerCameraManager;
  const SDK::FVector camera_location =
      camera_manager ? camera_manager->CameraCache.POV.Location : SDK::FVector{0.0f, 0.0f, 0.0f};
  const auto local_loc = ue4::GetActorLocation(static_cast<SDK::AActor *>(local_pawn));
  constexpr float kMaxDistanceCm = 45000.0f;
  constexpr float kMaxItemDistanceCm = 15000.0f;

  FrameContext ctx;
  ctx.controller = controller;
  ctx.local_pawn = static_cast<SDK::AActor *>(local_pawn);
  ctx.camera_manager = camera_manager;
  ctx.camera_location = camera_location;
  ctx.camera_valid = camera_location.X != 0.0f || camera_location.Y != 0.0f || camera_location.Z != 0.0f;
  ctx.local_pos = local_loc ? *local_loc : camera_location;
  ctx.view = view ? &*view : nullptr;
  ctx.local_team_id = ue4::player_data::ReadTeamId(reinterpret_cast<SDK::ASTExtraPlayerCharacter *>(local_pawn));
  ctx.item_lang = (ui::i18n::Current() == ui::i18n::Lang::En) ? 0 : 1;
  ctx.max_player_dist_sq = kMaxDistanceCm * kMaxDistanceCm;
  ctx.max_item_dist_sq = kMaxItemDistanceCm * kMaxItemDistanceCm;

  const bool show_deathbox = state::g_esp_deathbox.load(std::memory_order_relaxed);
  const bool show_loot = state::g_esp_loot.load(std::memory_order_relaxed);
  const bool show_vehicle = state::g_esp_vehicle.load(std::memory_order_relaxed);
  const bool show_items = show_deathbox || show_loot || show_vehicle;

  AimParams aim;
  aim.enabled = state::g_aimbot.load(std::memory_order_relaxed);
  aim.fov_radius = state::g_aim_fov.load(std::memory_order_relaxed);
  aim.max_distance = state::g_aim_distance.load(std::memory_order_relaxed);
  aim.ignore_bot = state::g_aim_ignore_bot.load(std::memory_order_relaxed);
  aim.ignore_knocked = state::g_aim_ignore_knocked.load(std::memory_order_relaxed);
  aim.bone = ResolveAimBone(state::g_aim_target_bone.load(std::memory_order_relaxed));

  std::vector<state::Player> player_frame;
  player_frame.reserve(40);
  std::vector<state::Item> item_frame;
  AimSelection sel;

  for (int i = 0; i < actors.Num(); ++i) {
    auto *actor = actors[i];
    if (!ptr::IsValid(actor)) {
      continue;
    }
    if (!ue4::character::IsPlayerCharacter(actor)) {
      if (show_items && ctx.view != nullptr && ctx.camera_valid && ue4::character::IsLikelyValidUObject(actor)) {
        CollectItemActor(actor, ctx, show_deathbox, show_loot, show_vehicle, item_frame);
      }
      continue;
    }
    if (actor == ctx.local_pawn) {
      continue;
    }

    auto *character = static_cast<SDK::ASTExtraPlayerCharacter *>(actor);
    PlayerResult r = BuildPlayer(character, ctx);
    if (!r.included) {
      continue;
    }
    if (aim.enabled) {
      ConsiderAimCandidate(character, r, ctx, aim, sel);
    }
    player_frame.push_back(std::move(r.player));
  }

  state::esp_buffer::Publish(std::move(player_frame));
  state::item_buffer::Publish(std::move(item_frame));

  if (aim.enabled && sel.found && ctx.view != nullptr) {
    ue4::aim::ApplyAimAssist(controller,
                             ctx.local_pawn,
                             sel.target,
                             sel.bone_world,
                             ctx.view->cam_loc,
                             sel.distance,
                             ctx.view);
  }
  const bool touch_steering =
      aim.enabled && sel.found && ctx.view != nullptr && state::g_aim_backend.load(std::memory_order_relaxed) == 0;
  if (!touch_steering) {
    ue4::touch_aim::Release();
  }
}

void hkProcessEvent(void *self, void *function, void *params) {
  if (IsReceiveDrawHUD(function)) {
    MXLOG_INFO_ONCE("hkProcessEvent: ReceiveDrawHUD matched (self=%p)", self);
    OnReceiveDrawHUD();
  }

  ProcessEventFn orig = g_original.load(std::memory_order_acquire);
  if (orig != nullptr) {
    orig(self, function, params);
  }
}

}  // namespace

bool IsInstalled() { return g_installed.load(std::memory_order_acquire); }

bool TryInstall() {
  if (g_installed.load(std::memory_order_acquire)) {
    return true;
  }
  auto *arr = SDK::UObject::GUObjectArray;
  if (arr == nullptr || SDK::FName::GNames == nullptr) {
    return false;
  }
  SDK::UClass *targets[kTargetCount] = {};
  int target_n = 0;
  for (int i = 0; i < kTargetCount; ++i) {
    if (auto *cls = SDK::UObject::FindClass(kTargetClasses[i])) {
      targets[target_n++] = cls;
    }
  }
  if (target_n == 0) {
    return false;
  }
  const int n = arr->ObjObjects.Num();
  int hooked = 0;
  for (int i = 0; i < n; ++i) {
    SDK::UObject *obj = arr->ObjObjects.GetByIndex(i);
    if (!ptr::IsValid(obj) || !ue4::character::IsLikelyValidUObject(obj)) {
      continue;
    }
    for (int t = 0; t < target_n; ++t) {
      if (!obj->IsA(targets[t])) {
        continue;
      }
      auto **vtab = *reinterpret_cast<void ***>(obj);
      if (ptr::IsValid(reinterpret_cast<uintptr_t>(vtab))) {
        void *cur_pe = vtab[kProcessEventSlot];
        if (ptr::IsValid(reinterpret_cast<uintptr_t>(cur_pe)) && cur_pe != reinterpret_cast<void *>(&hkProcessEvent)) {
          ProcessEventFn expected = nullptr;
          g_original.compare_exchange_strong(expected,
                                             reinterpret_cast<ProcessEventFn>(cur_pe),
                                             std::memory_order_acq_rel);
        }
      }

      const hook::SwapResult r =
          hook::SwapVTableSlot(obj, kProcessEventSlot, reinterpret_cast<void *>(&hkProcessEvent), nullptr);
      if (r == hook::SwapResult::kInstalled) {
        ++hooked;
      }
      break;
    }
  }

  if (hooked > 0) {
    g_installed.store(true, std::memory_order_release);
    MXLOG_INFO("pehook installed: hooked %d class vtable(s)", hooked);
    return true;
  }
  return false;
}

}  // namespace pehook
