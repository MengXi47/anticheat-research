#pragma once

#include "SDK/SDK.hpp"

namespace ue4::character {

bool IsLikelyValidUObject(SDK::UObject *obj);
bool IsPlayerCharacter(SDK::UObject *obj);

}  // namespace ue4::character
