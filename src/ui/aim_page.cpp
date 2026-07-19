#include "ui/aim_page.h"

#include <atomic>
#include <cfloat>

#include "imgui.h"
#include "state/aim_state.h"
#include "ui/i18n.h"
#include "ui/text_fx.h"
#include "ui/theme.h"
#include "ui/widgets.h"

namespace ui::aim_page {

namespace {

using i18n::T;
using widgets::CircleCheck;
using widgets::LineSlider;

const ImU32 kWhite = IM_COL32(255, 255, 255, 255);

ImU32 AccentA(float a) { return ImGui::GetColorU32(ImVec4(theme::kAccent.x, theme::kAccent.y, theme::kAccent.z, a)); }

void Header(const char *s) {
  constexpr float px = 20.0f;
  ImFont *font = ImGui::GetFont();
  const ImVec2 p = ImGui::GetCursorScreenPos();
  const ImVec2 ts = font->CalcTextSizeA(px, FLT_MAX, 0.0f, s);
  DrawBoldText(ImGui::GetWindowDrawList(), font, px, p, ImGui::GetColorU32(theme::kHeaderBlue), s);
  ImGui::Dummy(ImVec2(ts.x, ts.y + 8.0f));
}

void SliderRow(const char *id,
               const char *label,
               std::atomic<float> &value,
               float vmin,
               float vmax,
               const char *fmt,
               float w) {
  ImFont *font = ImGui::GetFont();
  const ImVec2 ts = font->CalcTextSizeA(theme::kToggleTextPx, FLT_MAX, 0.0f, label);
  DrawBoldText(ImGui::GetWindowDrawList(), font, theme::kToggleTextPx, ImGui::GetCursorScreenPos(), kWhite, label);
  ImGui::Dummy(ImVec2(ts.x, ts.y + 6.0f));

  float v = value.load(std::memory_order_relaxed);
  if (LineSlider(id, &v, vmin, vmax, fmt, w)) {
    value.store(v, std::memory_order_relaxed);
  }
  ImGui::Dummy(ImVec2(0, 10.0f));
}

void ComboRow(const char *id,
              const char *label,
              std::atomic<int> &value,
              const char *const *items,
              int count,
              float w) {
  ImFont *font = ImGui::GetFont();
  const ImVec2 ts = font->CalcTextSizeA(theme::kToggleTextPx, FLT_MAX, 0.0f, label);
  DrawBoldText(ImGui::GetWindowDrawList(), font, theme::kToggleTextPx, ImGui::GetCursorScreenPos(), kWhite, label);
  ImGui::Dummy(ImVec2(ts.x, ts.y + 2.0f));

  int cur = value.load(std::memory_order_relaxed);
  if (cur < 0 || cur >= count) {
    cur = 0;
  }

  ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(34, 38, 46, 240));
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(44, 49, 58, 245));
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(44, 49, 58, 245));
  ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(34, 38, 46, 240));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(44, 49, 58, 245));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(44, 49, 58, 245));
  ImGui::PushStyleColor(ImGuiCol_Text, kWhite);
  ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(28, 32, 40, 250));
  ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 255, 255, 40));
  ImGui::PushStyleColor(ImGuiCol_Header, AccentA(0.55f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, AccentA(0.35f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, AccentA(0.65f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));

  ImGui::SetNextItemWidth(w);
  if (ImGui::BeginCombo(id, items[cur])) {
    for (int i = 0; i < count; ++i) {
      const bool sel = (cur == i);
      if (ImGui::Selectable(items[i], sel)) {
        value.store(i, std::memory_order_relaxed);
      }
      if (sel) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor(12);
  ImGui::Dummy(ImVec2(0, 10.0f));
}

bool SegButton(const char *id, const char *label, bool active, float w) {
  constexpr float kH = 36.0f;
  const ImVec2 p0 = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton(id, ImVec2(w, kH));
  const bool clicked = ImGui::IsItemClicked();
  const bool hovered = ImGui::IsItemHovered();

  ImDrawList *dl = ImGui::GetWindowDrawList();
  const ImVec2 p1(p0.x + w, p0.y + kH);
  const ImU32 bg =
      active ? ImGui::GetColorU32(theme::kAccent) : (hovered ? IM_COL32(44, 49, 58, 245) : IM_COL32(34, 38, 46, 240));
  dl->AddRectFilled(p0, p1, bg, 8.0f);
  if (!active) {
    dl->AddRect(p0, p1, IM_COL32(255, 255, 255, 40), 8.0f, 0, 1.2f);
  }

  ImFont *font = ImGui::GetFont();
  const float px = theme::kToggleTextPx;
  const ImVec2 ts = font->CalcTextSizeA(px, FLT_MAX, 0.0f, label);
  DrawBoldText(dl, font, px, ImVec2(p0.x + (w - ts.x) * 0.5f, p0.y + (kH - ts.y) * 0.5f), kWhite, label);
  return clicked;
}

}  // namespace

void Draw() {
  constexpr float pad = 20.0f;
  ImGui::Dummy(ImVec2(0, 14));
  ImGui::Indent(pad);
  const float avail = ImGui::GetContentRegionAvail().x - pad;
  const float gap = 16.0f;
  const float col3_w = (avail - gap * 2.0f) / 3.0f;
  const float col_w = (avail - gap) * 0.5f;

  Header(T("自瞄", "Aimbot"));
  const float master_w = avail * 0.42f;
  const float seg_gap = 8.0f;
  const float seg_w = (avail - master_w - seg_gap) * 0.5f;
  CircleCheck("##chk_aimbot", T("總開關", "Master"), state::g_aimbot, master_w);
  ImGui::SameLine(0.0f, 0.0f);
  int backend = state::g_aim_backend.load(std::memory_order_relaxed);
  if (SegButton("##aim_touch", T("觸控", "Touch"), backend == 0, seg_w)) {
    backend = 0;
  }
  ImGui::SameLine(0.0f, seg_gap);
  if (SegButton("##aim_mem", T("內存", "Memory"), backend == 1, seg_w)) {
    backend = 1;
  }
  state::g_aim_backend.store(backend, std::memory_order_relaxed);
  ImGui::Dummy(ImVec2(0, 8));
  CircleCheck("##chk_fov_show", T("顯示範圍", "Show FOV"), state::g_aim_draw_fov, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck("##chk_ign_bot", T("略過人機", "Ignore Bots"), state::g_aim_ignore_bot, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck("##chk_ign_knock", T("略過倒地", "Ignore Knocked"), state::g_aim_ignore_knocked, col3_w);
  CircleCheck("##chk_predict", T("移動預測", "Prediction"), state::g_aim_prediction, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck("##chk_recoil_comp", T("彈道補償", "Bullet Drop"), state::g_aim_recoil_comp, col3_w);
  ImGui::SameLine(0.0f, gap);
  CircleCheck("##chk_no_recoil", T("無後座力", "No Recoil"), state::g_no_recoil, col3_w);
  ImGui::Dummy(ImVec2(0, 14));
  const char *bone_items[3] = {T("頭", "Head"), T("脖子", "Neck"), T("身", "Body")};
  const char *mode_items[4] = {T("一律", "Always"), T("開火", "Fire"), T("開鏡", "ADS"), T("混合", "Both")};
  ImGui::BeginGroup();
  ComboRow("##aim_bone", T("自瞄部位", "Aim Bone"), state::g_aim_target_bone, bone_items, 3, col_w);
  ImGui::EndGroup();
  ImGui::SameLine(0.0f, gap);
  ImGui::BeginGroup();
  ComboRow("##aim_mode", T("自瞄模式", "Aim Mode"), state::g_aim_method, mode_items, 4, col_w);
  ImGui::EndGroup();
  std::atomic<float> &aim_speed_var =
      (state::g_aim_backend.load(std::memory_order_relaxed) == 0) ? state::g_aim_speed_touch : state::g_aim_speed_mem;
  SliderRow("##aim_speed", T("自瞄速度", "Aim Speed"), aim_speed_var, 0.1f, 10.0f, "%.1f", avail);
  SliderRow("##aim_fov", T("自瞄範圍", "Aim FOV"), state::g_aim_fov, 10.0f, 400.0f, "%.0f", avail);
  SliderRow("##aim_dist", T("自瞄距離", "Aim Distance"), state::g_aim_distance, 5.0f, 400.0f, "%.0f m", avail);
  SliderRow("##aim_recoil_smooth", T("補償係數", "Drop Comp"), state::g_aim_recoil_smooth, 0.0f, 10.0f, "%.1f", avail);

  ImGui::Unindent(pad);
}

}  // namespace ui::aim_page
