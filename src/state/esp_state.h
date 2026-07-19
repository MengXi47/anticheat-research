#pragma once

#include <atomic>

#include "ue4/items_table.h"

namespace state {

inline std::atomic<bool> g_esp{true};
inline std::atomic<float> g_esp_line_thickness{1.0f};
inline std::atomic<float> g_screen_w{0.0f};
inline std::atomic<float> g_screen_h{0.0f};

inline std::atomic<bool> g_esp_line{true};
inline std::atomic<bool> g_esp_health{true};
inline std::atomic<bool> g_esp_skeleton{true};
inline std::atomic<bool> g_esp_name{true};
inline std::atomic<bool> g_esp_distance{true};
inline std::atomic<bool> g_esp_weapon{true};
inline std::atomic<bool> g_esp_hide_bots{false};
inline std::atomic<bool> g_esp_team_id{true};
inline std::atomic<bool> g_esp_show_count{true};
inline std::atomic<bool> g_esp_deathbox{false};
inline std::atomic<bool> g_esp_loot{false};
inline std::atomic<bool> g_esp_vehicle{false};
inline std::atomic<bool> g_esp_show_unknown{true};
inline std::atomic<bool> g_loot_cat[ue4::kItemCategoryCount];

namespace detail {

inline bool InitLootCategories() {
  constexpr bool kOn[ue4::kItemCategoryCount] = {
      true,   // Rifle
      true,   // SMG
      true,   // Sniper
      true,   // Shotgun
      true,   // LMG
      true,   // Pistol
      true,   // SpecialWeapon
      true,   // Melee
      false,  // Attachment
      false,  // Ammo
      true,   // Ghillie
      true,   // Armor
      true,   // Consumable
      true,   // Throwable
      false,  // Misc
      true,   // RareLoot
      true,   // Event
  };
  for (int i = 0; i < ue4::kItemCategoryCount; ++i) {
    g_loot_cat[i].store(kOn[i], std::memory_order_relaxed);
  }
  return true;
}

inline const bool kLootCategoriesInit = InitLootCategories();

}  // namespace detail

}  // namespace state
