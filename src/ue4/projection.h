#pragma once

#include <optional>

#include "SDK/SDK.hpp"
#include "imgui.h"

namespace ue4::projection {

struct ViewProjection {
  SDK::FVector cam_loc;
  SDK::FVector forward;
  SDK::FVector right;
  SDK::FVector up;
  float factor;
  float cx;
  float cy;
};

std::optional<ViewProjection> BuildViewProjection(SDK::APlayerController *controller);
ImVec2 Projection(const ViewProjection &vp, const SDK::FVector &world_pos);
std::optional<SDK::FVector> GetCameraLocation(SDK::APlayerController *controller);

}  // namespace ue4::projection
