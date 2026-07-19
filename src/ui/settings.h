#pragma once

#include <string>

namespace ui::settings {

constexpr int kSlotCount = 3;
void Init();
int Slot();
void SetSlot(int slot);
bool Save();
bool Load();
std::string FilePath(int slot);

} // namespace ui::settings
