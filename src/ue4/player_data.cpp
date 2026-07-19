#include "ue4/player_data.h"

#include <cstdint>

#include "common/ptr/valid.h"
#include "ue4/offset.h"

namespace ue4::player_data {

namespace {

namespace PC = ue4::pubg_offset;

constexpr std::uint32_t kFStringDataOffset = 0x00;
constexpr std::uint32_t kFStringNumOffset = 0x08;

constexpr int kMaxNameChars = 64;

std::string Utf16ToUtf8(const char16_t *data, int len) {
  std::string out;
  out.reserve(static_cast<std::size_t>(len) * 3);
  for (int i = 0; i < len; ++i) {
    std::uint32_t cp = data[i];
    if (cp == 0) {
      break;
    }
    if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < len) {
      const std::uint32_t lo = data[i + 1];
      if (lo >= 0xDC00 && lo <= 0xDFFF) {
        cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
        ++i;
      }
    }
    if (cp < 0x80) {
      out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
      out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
      out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
      out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
  }
  return out;
}

}  // namespace

Health ReadHealth(SDK::ASTExtraPlayerCharacter *character) {
  Health h;
  if (!ptr::IsValid(character)) {
    return h;
  }
  const auto base = reinterpret_cast<uintptr_t>(character);

  const float cur = *reinterpret_cast<float *>(base + PC::kHealth);
  const float max = *reinterpret_cast<float *>(base + PC::kHealthMax);
  h.max = max;
  h.current = (max < cur) ? max : cur;

  if (h.current > 0.0f) {
    return h;
  }

  h.current = 0.0f;
  const std::uint8_t dead = *reinterpret_cast<std::uint8_t *>(base + PC::kDeadFlag);
  if ((dead & 1) != 0) {
    h.dead = true;
    return h;
  }

  const float breath = *reinterpret_cast<float *>(base + PC::kNearDeathHP);
  h.current = breath;
  h.knocked = breath > 0.0f && breath < 100.0f;

  const auto ndc = *reinterpret_cast<uintptr_t *>(base + PC::kNearDeathComponent);
  if (ptr::IsValid(ndc)) {
    h.max = *reinterpret_cast<float *>(ndc + PC::kMaxBreath);
  }
  return h;
}

std::string ReadName(SDK::ASTExtraPlayerCharacter *character) {
  if (!ptr::IsValid(character)) {
    return {};
  }
  const auto fstr = reinterpret_cast<uintptr_t>(character) + PC::kPlayerNameFString;

  const auto data = *reinterpret_cast<char16_t **>(fstr + kFStringDataOffset);
  const std::int32_t num = *reinterpret_cast<std::int32_t *>(fstr + kFStringNumOffset);
  if (!ptr::IsValid(data) || num <= 0) {
    return {};
  }
  const int len = num < kMaxNameChars ? num : kMaxNameChars;
  return Utf16ToUtf8(data, len);
}

bool IsBot(SDK::ASTExtraPlayerCharacter *character) {
  if (!ptr::IsValid(character)) {
    return false;
  }
  const auto base = reinterpret_cast<uintptr_t>(character);
  return (*reinterpret_cast<std::uint8_t *>(base + PC::kbIsAI) & 1) != 0;
}

int ReadWeaponId(SDK::ASTExtraPlayerCharacter *character) {
  if (!ptr::IsValid(character)) {
    return 0;
  }
  const auto base = reinterpret_cast<uintptr_t>(character);

  const auto weapon_mgr = *reinterpret_cast<uintptr_t *>(base + PC::kWeaponManagerComponent);
  if (!ptr::IsValid(weapon_mgr)) {
    return 0;
  }
  const auto weapon = *reinterpret_cast<uintptr_t *>(weapon_mgr + PC::kWeaponMgrCurrentWeapon);
  if (!ptr::IsValid(weapon)) {
    return 0;
  }

  return reinterpret_cast<SDK::ASTExtraWeapon *>(weapon)->GetWeaponID();
}

int ReadTeamId(SDK::ASTExtraPlayerCharacter *character) {
  if (!ptr::IsValid(character)) {
    return -1;
  }
  const auto base = reinterpret_cast<uintptr_t>(character);
  return *reinterpret_cast<std::int32_t *>(base + PC::kTeamId);
}

}  // namespace ue4::player_data
