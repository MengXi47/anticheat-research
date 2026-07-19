#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "imgui.h"
#include "ue4/skeleton.h"

namespace state {

struct Player {
  struct Point {
    ImVec2 pos{};
    bool onscreen = false;
    bool visible = true;
  };
  std::array<Point, ue4::skeleton::kBoneCount> points{};
  ImU32 color = 0;

  float health = 0.0f;
  float health_max = 0.0f;
  bool knocked = false;
  float distance = 0.0f;
  std::string name;
  const char *weapon_name = nullptr;
  int team_id = 0;
  bool is_bot = false;
};

namespace esp_buffer {

void Publish(std::vector<Player> frame);
std::shared_ptr<const std::vector<Player>> Snapshot();
bool IsFresh(int max_age_ms);

}  // namespace esp_buffer

}  // namespace state
