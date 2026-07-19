#pragma once

namespace ue4 {

const char *GetItemNameByID(int items_id, int lang);
const char *VehicleName(int type, int lang);

enum class ItemCategory : int {
  Rifle = 0,
  SMG,
  Sniper,
  Shotgun,
  LMG,
  Pistol,
  SpecialWeapon,
  Melee,
  Attachment,
  Ammo,
  Ghillie,
  Armor,
  Consumable,
  Throwable,
  Misc,
  RareLoot,
  Event,
  Count
};

inline constexpr int kItemCategoryCount = static_cast<int>(ItemCategory::Count);

ItemCategory GetItemCategory(int items_id);

}  // namespace ue4
