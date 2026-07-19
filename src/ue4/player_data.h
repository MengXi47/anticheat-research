#pragma once

#include <string>

#include "SDK/SDK.hpp"

namespace ue4::player_data {

struct Health {
  float current = 0.0f;
  float max = 0.0f;
  bool knocked = false;
  bool dead = false;
};

Health ReadHealth(SDK::ASTExtraPlayerCharacter *character);
std::string ReadName(SDK::ASTExtraPlayerCharacter *character);
bool IsBot(SDK::ASTExtraPlayerCharacter *character);
int ReadWeaponId(SDK::ASTExtraPlayerCharacter *character);
int ReadTeamId(SDK::ASTExtraPlayerCharacter *character);

}  // namespace ue4::player_data
