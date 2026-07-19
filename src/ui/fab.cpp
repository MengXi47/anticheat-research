#include "ui/fab.h"

#include <algorithm>

#include "imgui.h"
#include "ui/icon.h"
#include "ui/menu.h"
#include "ui/theme.h"

namespace ui::fab {

namespace {

ImVec2 g_pos{50.0f, 80.0f};
bool g_moved_this_press = false;

} // namespace

void Draw() {
  ImGuiIO& io = ImGui::GetIO();
  g_pos.x = std::clamp(g_pos.x, 0.0f, io.DisplaySize.x - theme::kFabDiameter);
  g_pos.y = std::clamp(g_pos.y, 0.0f, io.DisplaySize.y - theme::kFabDiameter);

  ImGui::SetNextWindowPos(g_pos, ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(theme::kFabDiameter, theme::kFabDiameter));

  // clang-format off
  constexpr ImGuiWindowFlags kFlags = ImGuiWindowFlags_NoTitleBar       |
      ImGuiWindowFlags_NoResize     | ImGuiWindowFlags_NoMove           |
      ImGuiWindowFlags_NoCollapse   | ImGuiWindowFlags_NoScrollbar      |
      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings  |
      ImGuiWindowFlags_NoBringToFrontOnFocus;
  // clang-format on

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  if (ImGui::Begin("##fab", nullptr, kFlags)) {
    const ImVec2 pos = ImGui::GetWindowPos();

    ImGui::SetCursorScreenPos(pos);
    ImGui::InvisibleButton(
        "##hit", ImVec2(theme::kFabDiameter, theme::kFabDiameter));

    const bool active = ImGui::IsItemActive();

    if (ImGui::IsItemActivated()) {
      g_moved_this_press = false;
    }

    if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.0f)) {
      g_pos.x += io.MouseDelta.x;
      g_pos.y += io.MouseDelta.y;
      g_moved_this_press = true;
    }

    if (ImGui::IsItemDeactivated() && !g_moved_this_press) {
      menu::Toggle();
    }

    DrawMxIcon(pos, theme::kFabDiameter, theme::kFabDiameter * 0.5f, active);
  }
  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace ui::fab
