#include "ui/loot_page.h"

#include <cfloat>
#include <cstdio>

#include "imgui.h"
#include "state/esp_state.h"
#include "ue4/items_table.h"
#include "ui/i18n.h"
#include "ui/menu.h"
#include "ui/text_fx.h"
#include "ui/theme.h"
#include "ui/widgets.h"

namespace ui::loot_page {

namespace {

using i18n::T;
using widgets::CircleCheck;

const ImU32 kWhite = IM_COL32(255, 255, 255, 255);

struct CatLabel {
  const char* zh;
  const char* en;
};

constexpr CatLabel kCatLabels[] = {
    {"步槍", "Rifle"},
    {"衝鋒槍", "SMG"},
    {"狙擊槍", "Sniper"},
    {"霰彈槍", "Shotgun"},
    {"輕機槍", "LMG"},
    {"手槍", "Pistol"},
    {"特殊武器", "Special"},
    {"近戰", "Melee"},
    {"配件", "Attach"},
    {"彈藥", "Ammo"},
    {"吉利服", "Ghillie"},
    {"護甲", "Armor"},
    {"消耗品", "Consum."},
    {"投擲物", "Throw"},
    {"雜項", "Misc"},
    {"稀有", "Rare"},
    {"活動", "Event"},
};

static_assert(
    sizeof(kCatLabels) / sizeof(kCatLabels[0]) == ue4::kItemCategoryCount,
    "Category label count mismatch");

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

bool BackButton(const char* label) {
  ImFont* font = ImGui::GetFont();
  constexpr float px = 16.0f;
  const ImVec2 ts = font->CalcTextSizeA(px, FLT_MAX, 0.0f, label);
  const ImVec2 p0 = ImGui::GetCursorScreenPos();
  const ImVec2 sz(ts.x + 22.0f, ts.y + 12.0f);

  ImGui::InvisibleButton("##loot_back", sz);
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
  const float col3_w = (avail - gap * 2.0f) / 3.0f;

  if (BackButton(T("← 返回透視", "< Back to ESP"))) {
    ui::menu::OpenEspPage();
  }
  ImGui::Dummy(ImVec2(0, 10));

  Header(T("物資透視", "Loot ESP"));
  CircleCheck(
      "##chk_loot_master", T("總開關", "Master"), state::g_esp_loot, avail);
  ImGui::Dummy(ImVec2(0, 8));
  CircleCheck(
      "##chk_show_unknown",
      T("顯示未知物品(只有ID)", "Show Unknown (ID only)"),
      state::g_esp_show_unknown,
      avail);
  ImGui::Dummy(ImVec2(0, 8));

  Header(T("分類", "Categories"));
  for (int i = 0; i < ue4::kItemCategoryCount; ++i) {
    if (i % 3 != 0) {
      ImGui::SameLine(0.0f, gap);
    }
    char id[24];
    std::snprintf(id, sizeof(id), "##loot_cat_%d", i);
    CircleCheck(
        id,
        T(kCatLabels[i].zh, kCatLabels[i].en),
        state::g_loot_cat[i],
        col3_w);
  }

  ImGui::Unindent(pad);
}

} // namespace ui::loot_page
