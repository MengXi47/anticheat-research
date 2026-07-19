#include "ui/menu.h"

#include <algorithm>
#include <cfloat>

#include "imgui.h"
#include "state/texture.h"
#include "ui/aim_page.h"
#include "ui/esp_page.h"
#include "ui/home_page.h"
#include "ui/i18n.h"
#include "ui/icon.h"
#include "ui/loot_page.h"
#include "ui/text_fx.h"
#include "ui/theme.h"

namespace ui::menu {

namespace {

enum class Page { Home, Esp, Aim, Loot };

bool g_show_menu = false;
Page g_page = Page::Home;

constexpr float kCloseSize = 26.0f;
constexpr float kClosePad = 14.0f;

ImU32 Accent() {
  return ImGui::GetColorU32(theme::kAccent);
}
ImU32 AccentA(float a) {
  return ImGui::GetColorU32(
      ImVec4(theme::kAccent.x, theme::kAccent.y, theme::kAccent.z, a));
}

void DrawSidebarAvatar() {
  ImGui::Dummy(ImVec2(0, 4));
  const float d = theme::kMenuIconDiameter;
  const float avail = ImGui::GetContentRegionAvail().x;
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - d) * 0.5f);
  DrawMxIcon(
      ImGui::GetCursorScreenPos(),
      d,
      18.0f,
      /*tinted=*/false,
      /*border=*/false);
  ImGui::Dummy(ImVec2(0, d + 18.0f));
}

void DrawNavButton(
    void* icon, const char* id, const char* label, Page target, float w) {
  ImGui::SetCursorPosX(theme::kLeftPanelPadding);
  const ImVec2 p0 = ImGui::GetCursorScreenPos();
  const float h = theme::kNavButtonH;
  ImGui::InvisibleButton(id, ImVec2(w, h));
  const bool hovered = ImGui::IsItemHovered();
  if (ImGui::IsItemClicked()) {
    g_page = target;
  }
  const bool selected = (g_page == target);

  ImDrawList* dl = ImGui::GetWindowDrawList();
  const ImVec2 p1(p0.x + w, p0.y + h);
  const float pill = h * 0.5f;
  if (selected) {
    dl->AddRectFilled(p0, p1, AccentA(0.14f), pill);
    dl->AddRect(p0, p1, Accent(), pill, 0, 1.6f);
  } else if (hovered) {
    dl->AddRectFilled(p0, p1, IM_COL32(255, 255, 255, 22), pill);
  }

  if (icon) {
    constexpr float isz = 22.0f;
    const ImVec2 ic0(p0.x + 16.0f, p0.y + (h - isz) * 0.5f);
    const ImVec2 ic1(ic0.x + isz, ic0.y + isz);
    const ImU32 tint = selected ? Accent()
        : hovered
        ? IM_COL32(255, 255, 255, 255)
        : IM_COL32(208, 212, 220, 255);
    dl->AddImage(
        reinterpret_cast<ImTextureID>(icon),
        ic0,
        ic1,
        ImVec2(0, 0),
        ImVec2(1, 1),
        tint);
  }

  ImFont* font = ImGui::GetFont();
  const ImVec2 ts =
      font->CalcTextSizeA(theme::kNavTextPx, FLT_MAX, 0.0f, label);
  const ImU32 txt =
      selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(212, 216, 222, 255);
  DrawBoldText(
      dl,
      font,
      theme::kNavTextPx,
      ImVec2(p0.x + 48.0f, p0.y + (h - ts.y) * 0.5f),
      txt,
      label);

  ImGui::Dummy(ImVec2(0, theme::kNavButtonGap));
}

void DrawLeftPanel() {
  ImGui::PushStyleColor(ImGuiCol_ChildBg, theme::kBgPanelL);
  ImGui::BeginChild(
      "##left",
      ImVec2(theme::kLeftPanelW, 0),
      /*border=*/false,
      ImGuiWindowFlags_NoScrollbar);

  DrawSidebarAvatar();
  const float button_w = theme::kLeftPanelW - 2.0f * theme::kLeftPanelPadding;
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
  DrawNavButton(
      gfx::HomeIcon(),
      "##nav_home",
      i18n::T("首頁", "HOME"),
      Page::Home,
      button_w);
  DrawNavButton(
      gfx::VisionIcon(),
      "##nav_visual",
      i18n::T("透視", "ESP"),
      Page::Esp,
      button_w);
  DrawNavButton(
      gfx::TargetIcon(),
      "##nav_aim",
      i18n::T("自瞄", "AIMBOT"),
      Page::Aim,
      button_w);
  ImGui::PopStyleVar();

  ImGui::EndChild();
  ImGui::PopStyleColor();
}

void DrawRightPanel() {
  ImGui::PushStyleColor(ImGuiCol_ChildBg, theme::kBgPanelR);
  ImGui::BeginChild("##right", ImVec2(0, 0), /*border=*/false);
  switch (g_page) {
    case Page::Home:
      home_page::Draw();
      break;
    case Page::Esp:
      esp_page::Draw();
      break;
    case Page::Aim:
      aim_page::Draw();
      break;
    case Page::Loot:
      loot_page::Draw();
      break;
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();
}

void PushMenuStyle() {
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
  ImGui::PushStyleColor(ImGuiCol_Text, theme::kText);
  ImGui::PushStyleColor(ImGuiCol_TextDisabled, theme::kTextDim);
  ImGui::PushStyleColor(ImGuiCol_Separator, theme::kSeparator);
  ImGui::PushStyleColor(ImGuiCol_Button, theme::kBtn);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme::kBtnHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme::kBtnHover);
  ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, theme::kScrollBg);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
}

void PopMenuStyle() {
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(8);
}

constexpr ImU32 kBgTint = IM_COL32(120, 120, 120, 255);

void DrawBackground() {
  void* tex = gfx::BackgroundTexture();
  if (!tex) {
    return;
  }
  const ImVec2 p_min = ImGui::GetWindowPos();
  const ImVec2 size = ImGui::GetWindowSize();
  const ImVec2 p_max(p_min.x + size.x, p_min.y + size.y);
  ImGui::GetWindowDrawList()->AddImageRounded(
      reinterpret_cast<ImTextureID>(tex),
      p_min,
      p_max,
      ImVec2(0, 0),
      ImVec2(1, 1),
      kBgTint,
      ImGui::GetStyle().WindowRounding,
      ImDrawFlags_RoundCornersAll);
}

void DrawTitle() {
  const ImVec2 wp = ImGui::GetWindowPos();
  const float ww = ImGui::GetWindowSize().x;
  ImFont* font = ImGui::GetFont();
  const char* kTitle = "MengXi Game";
  const ImVec2 ts =
      font->CalcTextSizeA(theme::kBannerTextPx, FLT_MAX, 0.0f, kTitle);
  const float cy = wp.y + kClosePad + kCloseSize * 0.5f;
  const ImVec2 pos(wp.x + (ww - ts.x) * 0.5f, cy - ts.y * 0.5f);
  DrawBoldText(
      ImGui::GetWindowDrawList(),
      font,
      theme::kBannerTextPx,
      pos,
      IM_COL32(255, 255, 255, 255),
      kTitle);
  ImGui::Dummy(ImVec2(0, theme::kBannerHeight));
}

bool DrawCloseButton() {
  const ImVec2 wp = ImGui::GetWindowPos();
  const ImVec2 ws = ImGui::GetWindowSize();
  const ImVec2 tl(wp.x + ws.x - kClosePad - kCloseSize, wp.y + kClosePad);
  const ImVec2 br(tl.x + kCloseSize, tl.y + kCloseSize);
  const ImVec2 c(tl.x + kCloseSize * 0.5f, tl.y + kCloseSize * 0.5f);

  const bool hovered = ImGui::IsMouseHoveringRect(tl, br);
  const bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

  ImDrawList* dl = ImGui::GetForegroundDrawList();
  if (hovered) {
    dl->AddCircleFilled(c, kCloseSize * 0.5f, IM_COL32(255, 255, 255, 28), 24);
  }
  const ImU32 col =
      hovered ? IM_COL32(255, 80, 80, 255) : IM_COL32(220, 80, 80, 255);
  const float k = kCloseSize * 0.26f;
  dl->AddLine(ImVec2(c.x - k, c.y - k), ImVec2(c.x + k, c.y + k), col, 2.6f);
  dl->AddLine(ImVec2(c.x - k, c.y + k), ImVec2(c.x + k, c.y - k), col, 2.6f);

  return clicked;
}

} // namespace

void Draw() {
  if (!g_show_menu) {
    return;
  }
  const ImGuiIO& io = ImGui::GetIO();
  const ImVec2 init_size{
      std::clamp(io.DisplaySize.x * 0.66f, 580.0f, 1080.0f),
      std::clamp(io.DisplaySize.y * 0.64f, 380.0f, 720.0f),
  };
  const ImVec2 init_pos{
      std::max((io.DisplaySize.x - init_size.x) * 0.5f, 0.0f),
      std::max((io.DisplaySize.y - init_size.y) * 0.5f, 0.0f),
  };
  ImGui::SetNextWindowPos(init_pos, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(init_size, ImGuiCond_FirstUseEver);

  PushMenuStyle();

  constexpr ImGuiWindowFlags kMenuFlags = ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings;

  bool open = true;
  bool close_clicked = false;
  if (ImGui::Begin("##pubg_mx_menu", &open, kMenuFlags)) {
    DrawBackground();
    DrawTitle();

    DrawLeftPanel();
    const ImVec2 lmin = ImGui::GetItemRectMin();
    const ImVec2 lmax = ImGui::GetItemRectMax();
    // 側欄與內容間的垂直分隔線。
    const float dx = lmax.x + theme::kCardSpacing * 0.5f;
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(dx, lmin.y + 4.0f),
        ImVec2(dx, lmax.y - 4.0f),
        IM_COL32(255, 255, 255, 38),
        1.0f);

    ImGui::SameLine(0.0f, theme::kCardSpacing);
    DrawRightPanel();

    close_clicked = DrawCloseButton();
  }
  ImGui::End();
  PopMenuStyle();

  if (!open || close_clicked) {
    g_show_menu = false;
  }
}

void Toggle() {
  g_show_menu = !g_show_menu;
}

bool IsVisible() {
  return g_show_menu;
}

void OpenLootPage() {
  g_page = Page::Loot;
}

void OpenEspPage() {
  g_page = Page::Esp;
}

} // namespace ui::menu
