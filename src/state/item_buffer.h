#pragma once

#include <memory>
#include <vector>

#include "imgui.h"

namespace state {

inline constexpr int kCategoryDeathBox = -1;
inline constexpr int kCategoryVehicle = -2;
inline constexpr int kCategoryUnknownItem = -3;

struct Item {
  ImVec2 pos{};
  bool onscreen = false;
  float distance = 0.0f;
  int category = 0;
  const char *name = nullptr;
  int id = 0;
};

namespace item_buffer {

void Publish(std::vector<Item> frame);
std::shared_ptr<const std::vector<Item>> Snapshot();
bool IsFresh(int max_age_ms);

}  // namespace item_buffer

}  // namespace state
