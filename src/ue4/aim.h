#pragma once

#include "SDK/SDK.hpp"

namespace ue4::projection {
struct ViewProjection;
}  // namespace ue4::projection

namespace ue4::aim {

void NormalizeDelta(SDK::FRotator &delta);
SDK::FRotator LookAt(const SDK::FVector &start, const SDK::FVector &target);
void ApplyAimAssist(SDK::APlayerController *controller,
                    SDK::AActor *local_pawn,
                    SDK::AActor *target,
                    SDK::FVector aim_point,
                    const SDK::FVector &cam_loc,
                    float distance_m,
                    const ue4::projection::ViewProjection *view);
void ApplyNoRecoil(SDK::AActor *local_pawn);

}  // namespace ue4::aim
