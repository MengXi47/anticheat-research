#include "ui/esp_page.h"

#include <cfloat>

#include "imgui.h"
#include "state/esp_state.h"
#include "ui/i18n.h"
#include "ui/menu.h"
#include "ui/text_fx.h"
#include "ui/theme.h"
#include "ui/widgets.h"

namespace ui::esp_page {

namespace {

using i18n::T;
using widgets::CircleCheck;
using widgets::LineSlider;

const ImU32 kWhite = IM_COL32(255, 255, 255, 255);

void Header(const char* s) {
  constexpr float px = 20.0f;
  ImFont* font = ImGui::GetFont();
  const ImVec2 p = ImGui::GetCursorScreenPos();
  const ImVec2 ts = font->CalcTextSizeA(px, FLT_MAX, 0.0f, s);
  DrawBoldText(
      ImGui::GetWindowDrawList(),
      font,
      px,
      p,
      ImGui::GetColorU32(theme::kHeaderBlue),
      s);
  ImGui::Dummy(ImVec2(ts.x, ts.y + 8.0f));
}

void Label(const char* s, float px) {
  ImFont* font = ImGui::GetFont();
  const ImVec2 ts = font->CalcTextSizeA(px, FLT_MAX, 0.0f, s);
  DrawBoldText(
      ImGui::GetWindowDrawList(),
      font,
      px,
      ImGui::GetCursorScreenPos(),
      kWhite,
      s);
  ImGui::Dummy(ts);
}

bool NavButton(const char* id, const char* label) {
  ImFont* font = ImGui::GetFont();
  constexpr float px = 16.0f;
  const ImVec2 ts = font->CalcTextSizeA(px, FLT_MAX, 0.0f, label);
  const ImVec2 p0 = ImGui::GetCursorScreenPos();
  const ImVec2 sz(ts.x + 22.0f, ts.y + 12.0f);

  ImGui::InvisibleButton(id, sz);
  const bool hovered = ImGui::IsItemHovered();
  const bool clicked = ImGui::IsItemClicked();

  ImDrawList* dl = ImGui::GetWindowDrawList();
  const float r = sz.y * 0.5f;
  dl->AddRectFilled(
      p0,
      ImVec2(p0.x + sz.x, p0.y + sz.y),
      hovered ? IM_COL32(255, 255, 255, 36) : IM_COL32(255, 255, 255, 18),
      r);
  DrawBoldText(dl, font, px, ImVec2(p0.x + 11.0f, p0.y + 6.0f), kWhite, label);
  return clicked;
}

} // namespace

void Draw() {
  constexpr float pad = 20.0f;
  ImGui::Dummy(ImVec2(0, 14));
  ImGui::Indent(pad);
  const float avail = ImGui::GetContentRegionAvail().x - pad;
  const float gap = 16.0f;
  const float col_w = (avail - gap) * 0.5f;
  const float col3_w = (avail - gap * 2.0f) / 3.0f;

  const char* loot_label = T("物資 ESP →", "Loot ESP >");
  const float loot_w =
      ImGui::GetFont()->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, loot_label).x +
      22.0f;
  Header(T("透視", "ESP"));
  ImGui::SameLine();
  ImGui::SetCursorPosX(
      ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - loot_w);
  if (NavButton("##to_loot", loot_label)) {
    ui::menu::OpenLootPage();
  }
  CircleCheck("##chk_master", T("總開關", "Master"), state::g_esp, avail);
  ImGui::Dummy(ImVec2(0, 8));

  Header(T("項目", "Items"));
  CircleCheck("##chk_line", T("線條", "Lines"), state::g_esp_line, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck("##chk_health", T("血量", "Health"), state::g_esp_health, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck(
      "##chk_skeleton", T("骨骼", "Skeleton"), state::g_esp_skeleton, col3_w);

  CircleCheck("##chk_name", T("名字", "Name"), state::g_esp_name, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck(
      "##chk_distance", T("距離", "Distance"), state::g_esp_distance, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck("##chk_weapon", T("武器", "Weapon"), state::g_esp_weapon, col3_w);
  CircleCheck(
      "##chk_hidebots",
      T("隱藏機器人", "Hide Bots"),
      state::g_esp_hide_bots,
      col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck(
      "##chk_teamid", T("隊伍ID", "Team ID"), state::g_esp_team_id, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck(
      "##chk_count",
      T("顯示人數", "Show Count"),
      state::g_esp_show_count,
      col3_w);
  CircleCheck(
      "##chk_deathbox",
      T("死亡盒", "Death Box"),
      state::g_esp_deathbox,
      col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck(
      "##chk_vehicle", T("載具", "Vehicle"), state::g_esp_vehicle, col3_w);
  ImGui::Dummy(ImVec2(0, 16));
  Label(T("線條粗細", "Line Width"), theme::kToggleTextPx);
  ImGui::Dummy(ImVec2(0, 6));
  float thickness = state::g_esp_line_thickness.load(std::memory_order_relaxed);
  if (LineSlider("##thickness", &thickness, 1.0f, 5.0f, "%.1f", avail)) {
    state::g_esp_line_thickness.store(thickness, std::memory_order_relaxed);
  }

  ImGui::Unindent(pad);
}

} // namespace ui::esp_page
