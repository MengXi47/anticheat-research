#include "ue4/aim.h"

#include <array>
#include <cmath>
#include <cstdint>

#include "common/ptr/valid.h"
#include "state/aim_state.h"
#include "state/esp_state.h"
#include "ue4/offset.h"
#include "ue4/projection.h"
#include "ue4/touch_aim.h"

namespace ue4::aim {

namespace {
constexpr float kRad2Deg = 57.295780f;
constexpr float kPitchClamp = 75.0f;
}  // namespace

void NormalizeDelta(SDK::FRotator &delta) {
  float pitch = delta.Pitch;
  if (pitch > 180.0f) {
    pitch -= 360.0f;
  } else if (pitch < -180.0f) {
    pitch += 360.0f;
  }
  if (pitch < -kPitchClamp) {
    pitch = -kPitchClamp;
  } else if (pitch > kPitchClamp) {
    pitch = kPitchClamp;
  }
  delta.Pitch = pitch;
  float yaw = delta.Yaw;
  while (yaw < -180.0f) {
    yaw += 360.0f;
  }
  while (yaw > 180.0f) {
    yaw -= 360.0f;
  }
  delta.Yaw = yaw;
}

SDK::FRotator LookAt(const SDK::FVector &start, const SDK::FVector &target) {
  const float dx = target.X - start.X;
  const float dy = target.Y - start.Y;
  const float dz = target.Z - start.Z;

  const float horiz = std::sqrt(dx * dx + dy * dy);

  SDK::FRotator r{};
  r.Yaw = std::atan2(dy, dx) * kRad2Deg;
  r.Pitch = std::atan2(dz, horiz) * kRad2Deg;
  r.Roll = 0.0f;

  return r;
}

namespace {

namespace PC = ue4::pubg_offset;

SDK::FRotator VectorToRotator(const SDK::FVector &dir) {
  SDK::FRotator r{};
  r.Yaw = std::atan2(dir.Y, dir.X) * kRad2Deg;
  r.Pitch = std::atan2(dir.Z, std::sqrt(dir.X * dir.X + dir.Y * dir.Y)) * kRad2Deg;
  r.Roll = 0.0f;
  return r;
}

bool MethodTriggered(SDK::AActor *local_pawn) {
  const int method = state::g_aim_method.load(std::memory_order_relaxed);
  if (method == 0) {
    return true;
  }
  const auto base = reinterpret_cast<std::uintptr_t>(local_pawn);
  const bool firing = (*reinterpret_cast<std::uint8_t *>(base + PC::kbIsWeaponFiring) & 1) != 0;
  const bool ads = (*reinterpret_cast<std::uint8_t *>(base + PC::kbIsGunADS) & 1) != 0;
  switch (method) {
    case 1:
      return firing;
    case 2:
      return ads;
    case 3:
      return firing && ads;
    default:
      return true;
  }
}

bool IsAdsAndFiring(SDK::AActor *local_pawn) {
  const auto base = reinterpret_cast<std::uintptr_t>(local_pawn);
  const bool ads = (*reinterpret_cast<std::uint8_t *>(base + PC::kbIsGunADS) & 1) != 0;
  const bool firing = (*reinterpret_cast<std::uint8_t *>(base + PC::kbIsWeaponFiring) & 1) != 0;
  return ads && firing;
}

std::uintptr_t CurrentWeaponObj(SDK::AActor *local_pawn) {
  const auto lbase = reinterpret_cast<std::uintptr_t>(local_pawn);
  auto *wmgr_raw = *reinterpret_cast<void **>(lbase + PC::kWeaponMgr);
  if (wmgr_raw == nullptr) {
    return 0;
  }
  auto *wmgr = static_cast<SDK::UWeaponManagerComponent *>(wmgr_raw);
  const auto slot_raw = wmgr->GetCurrentUsingPropSlot();
  const unsigned slot = static_cast<unsigned>(reinterpret_cast<const std::uint8_t &>(slot_raw));
  if (slot - 1u > 2u) {
    return 0;
  }
  auto *s1 = *reinterpret_cast<void **>(reinterpret_cast<std::uintptr_t>(wmgr_raw) + PC::kWeaponMgrSlotPtr);
  if (s1 == nullptr) {
    return 0;
  }
  auto *s2 = *reinterpret_cast<void **>(reinterpret_cast<std::uintptr_t>(s1) + PC::kWeaponSlotInner);
  if (s2 == nullptr) {
    return 0;
  }
  auto *wobj = *reinterpret_cast<void **>(reinterpret_cast<std::uintptr_t>(s2) + PC::kWeaponObjPtr);
  return reinterpret_cast<std::uintptr_t>(wobj);
}

SDK::FVector PredictLead(SDK::AActor *local_pawn,
                         SDK::AActor *target,
                         const SDK::FVector &shooter_pos,
                         const SDK::FVector &target_pos) {
  const SDK::FVector zero{0.0f, 0.0f, 0.0f};
  const std::uintptr_t wobj = CurrentWeaponObj(local_pawn);
  if (wobj == 0) {
    return zero;
  }
  const float bullet_speed = *reinterpret_cast<float *>(wobj + PC::kBulletSpeed);
  if (bullet_speed <= 1.0f) {
    return zero;
  }
  SDK::FVector vel{};
  auto *veh = *reinterpret_cast<void **>(reinterpret_cast<std::uintptr_t>(target) + PC::kTargetVelOverride);
  if (veh != nullptr) {
    vel = *reinterpret_cast<SDK::FVector *>(reinterpret_cast<std::uintptr_t>(veh) + PC::kVelOverrideXYZ);
  } else {
    vel = target->GetVelocity();
  }
  constexpr int kInterceptIterations = 3;
  const float dx = target_pos.X - shooter_pos.X;
  const float dy = target_pos.Y - shooter_pos.Y;
  const float dz = target_pos.Z - shooter_pos.Z;
  float t = std::sqrt(dx * dx + dy * dy + dz * dz) / bullet_speed;
  for (int i = 0; i < kInterceptIterations; ++i) {
    const float fx = dx + vel.X * t;
    const float fy = dy + vel.Y * t;
    const float fz = dz + vel.Z * t;
    t = std::sqrt(fx * fx + fy * fy + fz * fz) / bullet_speed;
  }
  return SDK::FVector{vel.X * t, vel.Y * t, vel.Z * t};
}

constexpr std::uint32_t kRecoilFields[] = {
    PC::kAccVRecoilFactor,
    PC::kAccRecoveryFactor,
    PC::kAccHRecoilFactor,
    PC::kRecoilKick,
    PC::kRecoilKickADS,
    PC::kAnimationKick,
};
constexpr int kRecoilFieldCount = std::size(kRecoilFields);

struct RecoilBackup {
  std::uintptr_t wobj = 0;
  std::array<float, kRecoilFieldCount> values{};
};
RecoilBackup g_recoil_backup;

}  // namespace

void ApplyAimAssist(SDK::APlayerController *controller,
                    SDK::AActor *local_pawn,
                    SDK::AActor *target,
                    SDK::FVector aim_point,
                    const SDK::FVector &cam_loc,
                    float distance_m,
                    const ue4::projection::ViewProjection *view) {
  const bool is_touch = state::g_aim_backend.load(std::memory_order_relaxed) == 0;
  if (!ptr::IsValid(controller) || !ptr::IsValid(local_pawn)) {
    if (is_touch) {
      ue4::touch_aim::Release();
    }
    return;
  }
  if (!MethodTriggered(local_pawn)) {
    if (is_touch) {
      ue4::touch_aim::Release();
    }
    return;
  }

  const SDK::FVector target_pos = aim_point;
  if (state::g_aim_recoil_comp.load(std::memory_order_relaxed) && IsAdsAndFiring(local_pawn)) {
    aim_point.Z -= distance_m * state::g_aim_recoil_smooth.load(std::memory_order_relaxed);
  }
  if (state::g_aim_prediction.load(std::memory_order_relaxed) && ptr::IsValid(target)) {
    const SDK::FVector lead = PredictLead(local_pawn, target, cam_loc, target_pos);
    aim_point.X += lead.X;
    aim_point.Y += lead.Y;
    aim_point.Z += lead.Z;
  }
  if (is_touch) {
    if (view == nullptr) {
      ue4::touch_aim::Release();
      return;
    }
    const ImVec2 scr = ue4::projection::Projection(*view, aim_point);
    const float W = state::g_screen_w.load(std::memory_order_relaxed);
    const float H = state::g_screen_h.load(std::memory_order_relaxed);
    const bool reject = !std::isfinite(scr.x) || !std::isfinite(scr.y) || (scr.x == 0.0f && scr.y == 0.0f) ||
                        scr.x < 0.0f || scr.y < 0.0f || (W > 0.0f && scr.x > W) || (H > 0.0f && scr.y > H);
    if (reject) {
      ue4::touch_aim::Release();
      return;
    }
    const float dx = scr.x - view->cx;
    const float dy = scr.y - view->cy;
    if (!std::isfinite(dx * dx + dy * dy)) {
      ue4::touch_aim::Release();
      return;
    }
    constexpr float kTouchSpeedScale = 0.3f;
    float div = state::g_aim_speed_touch.load(std::memory_order_relaxed);
    if (div < 0.1f) {
      div = 0.7f;
    }
    ue4::touch_aim::Steer(scr.x, scr.y, view->cx, view->cy, kTouchSpeedScale / div);
    return;
  }

  const SDK::FVector dir{aim_point.X - cam_loc.X, aim_point.Y - cam_loc.Y, aim_point.Z - cam_loc.Z};
  const SDK::FRotator tgt_rot = VectorToRotator(dir);

  auto *mgr = controller->PlayerCameraManager;
  if (!ptr::IsValid(mgr)) {
    return;
  }
  const SDK::FRotator current = mgr->CameraCache.POV.Rotation;
  SDK::FRotator delta{};
  delta.Pitch = tgt_rot.Pitch - current.Pitch;
  delta.Yaw = tgt_rot.Yaw - current.Yaw;
  delta.Roll = 0.0f;
  NormalizeDelta(delta);
  float speed = state::g_aim_speed_mem.load(std::memory_order_relaxed);
  if (speed < 1.0f) {
    speed = 0.7f;
  }
  auto *pawn = static_cast<SDK::APawn *>(local_pawn);
  pawn->AddControllerYawInput(delta.Yaw / speed);
  pawn->AddControllerPitchInput(delta.Pitch / speed);
}

void ApplyNoRecoil(SDK::AActor *local_pawn) {
  const bool enabled = state::g_no_recoil.load(std::memory_order_relaxed);
  if (!enabled && g_recoil_backup.wobj == 0) {
    return;
  }
  if (!ptr::IsValid(local_pawn)) {
    return;
  }
  const std::uintptr_t wobj = CurrentWeaponObj(local_pawn);

  if (!enabled) {
    if (wobj != 0 && g_recoil_backup.wobj == wobj) {
      for (int i = 0; i < kRecoilFieldCount; ++i) {
        *reinterpret_cast<float *>(wobj + kRecoilFields[i]) = g_recoil_backup.values[i];
      }
      g_recoil_backup.wobj = 0;
    }
    return;
  }

  if (wobj == 0) {
    return;
  }

  if (g_recoil_backup.wobj != wobj) {
    g_recoil_backup.wobj = wobj;
    for (int i = 0; i < kRecoilFieldCount; ++i) {
      g_recoil_backup.values[i] = *reinterpret_cast<float *>(wobj + kRecoilFields[i]);
    }
  }

  for (int i = 0; i < kRecoilFieldCount; ++i) {
    *reinterpret_cast<float *>(wobj + kRecoilFields[i]) = 0.0f;
  }
}

}  // namespace ue4::aim
