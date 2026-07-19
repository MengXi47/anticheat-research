#include "projection.h"

#include <cmath>

#include "common/log/log.h"
#include "common/ptr/valid.h"
#include "state/esp_state.h"

namespace ue4::projection {

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kDegToRad = kPi / 180.0f;

struct Basis {
  SDK::FVector forward;
  SDK::FVector right;
  SDK::FVector up;
};

inline Basis ComputeBasis(const SDK::FRotator &rot) {
  const float p = rot.Pitch * kDegToRad;
  const float y = rot.Yaw * kDegToRad;
  const float r = rot.Roll * kDegToRad;
  const float sp = std::sin(p), cp = std::cos(p);
  const float sy = std::sin(y), cy = std::cos(y);
  const float sr = std::sin(r), cr = std::cos(r);

  return Basis{
      .forward = {cp * cy, cp * sy, sp},
      .right = {sp * sr * cy - cr * sy, sp * sr * sy + cr * cy, -sr * cp},
      .up = {-(sp * cr * cy + sr * sy), sr * cy - sp * cr * sy, cp * cr},
  };
}

inline float Dot(const SDK::FVector &a, const SDK::FVector &b) { return a.X * b.X + a.Y * b.Y + a.Z * b.Z; }

}  // namespace

std::optional<ViewProjection> BuildViewProjection(SDK::APlayerController *controller) {
  if (!ptr::IsValid(controller)) {
    MXLOG_ERROR_ONCE("ue4::projection::BuildViewProjection: controller invalid (%p)", static_cast<void *>(controller));
    return std::nullopt;
  }
  auto *mgr = controller->PlayerCameraManager;
  if (!ptr::IsValid(mgr)) {
    MXLOG_WARN_ONCE("W2S: PlayerCameraManager null (%p)", static_cast<void *>(mgr));
    return std::nullopt;
  }

  const SDK::FMinimalViewInfo &pov = mgr->CameraCache.POV;
  const SDK::FRotator &cam_rot = pov.Rotation;
  const float fov_deg = pov.FOV;
  if (fov_deg < 1.0f || fov_deg > 179.0f) {
    return std::nullopt;
  }

  const float disp_w = state::g_screen_w.load(std::memory_order_relaxed);
  const float disp_h = state::g_screen_h.load(std::memory_order_relaxed);
  if (disp_w < 1.0f || disp_h < 1.0f) {
    return std::nullopt;
  }
  const float tan_h = std::tan(fov_deg * 0.5f * kDegToRad);
  if (tan_h < 0.0001f) {
    return std::nullopt;
  }

  const Basis basis = ComputeBasis(cam_rot);
  const float cx = disp_w * 0.5f;
  const float cy = disp_h * 0.5f;

  return ViewProjection{
      .cam_loc = pov.Location,
      .forward = basis.forward,
      .right = basis.right,
      .up = basis.up,
      .factor = cx / tan_h,
      .cx = cx,
      .cy = cy,
  };
}

ImVec2 Projection(const ViewProjection &vp, const SDK::FVector &world_pos) {
  const SDK::FVector delta{
      world_pos.X - vp.cam_loc.X,
      world_pos.Y - vp.cam_loc.Y,
      world_pos.Z - vp.cam_loc.Z,
  };

  const float fwd_dot = Dot(delta, vp.forward);
  const float depth = fwd_dot < 1.0f ? 1.0f : fwd_dot;
  const float right_dot = Dot(delta, vp.right);
  const float up_dot = Dot(delta, vp.up);

  return ImVec2{
      right_dot * vp.factor / depth + vp.cx,
      vp.cy - up_dot * vp.factor / depth,
  };
}

std::optional<SDK::FVector> GetCameraLocation(SDK::APlayerController *controller) {
  if (!ptr::IsValid(controller)) {
    return std::nullopt;
  }
  auto *mgr = controller->PlayerCameraManager;
  if (!ptr::IsValid(mgr)) {
    return std::nullopt;
  }
  return mgr->CameraCache.POV.Location;
}

}  // namespace ue4::projection
