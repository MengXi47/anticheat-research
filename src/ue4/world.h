#pragma once

#include <optional>

#include "SDK/SDK.hpp"

namespace ue4 {

SDK::UWorld *GetWorld();
SDK::TArray<SDK::AActor *> GetActors(SDK::ULevel *level);
SDK::APlayerController *GetLocalPlayerController(SDK::UWorld *world);
SDK::APawn *GetLocalPawn(SDK::APlayerController *controller);
std::optional<SDK::FVector> GetActorLocation(SDK::AActor *actor);

}  // namespace ue4
