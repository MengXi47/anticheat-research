#include "ui/home_page.h"

#include <cfloat>
#include <cstdio>

#include <mach/mach.h>
#include <sys/sysctl.h>

#include "imgui.h"
#include "state/app_state.h"
#include "ui/i18n.h"
#include "ui/settings.h"
#include "ui/text_fx.h"
#include "ui/theme.h"
#include "ui/widgets.h"

namespace ui::home_page {

namespace {

using i18n::T;

const ImU32 kWhite = IM_COL32(255, 255, 255, 255);

ImU32 AccentA(float a) {
  return ImGui::GetColorU32(
      ImVec4(theme::kAccent.x, theme::kAccent.y, theme::kAccent.z, a));
}

bool ActionButton(const char* id, const char* label, float w) {
  constexpr float h = 38.0f;
  const ImVec2 p0 = ImGui::GetCursorScreenPos();
  ImGui::InvisibleButton(id, ImVec2(w, h));
  const bool hovered = ImGui::IsItemHovered();
  const bool clicked = ImGui::IsItemClicked();
  ImDrawList* dl = ImGui::GetWindowDrawList();
  const float r = h * 0.5f;
  dl->AddRectFilled(
      p0, ImVec2(p0.x + w, p0.y + h), AccentA(hovered ? 0.9f : 0.7f), r);
  ImFont* font = ImGui::GetFont();
  const ImVec2 ts =
      font->CalcTextSizeA(theme::kToggleTextPx, FLT_MAX, 0.0f, label);
  DrawBoldText(
      dl,
      font,
      theme::kToggleTextPx,
      ImVec2(p0.x + (w - ts.x) * 0.5f, p0.y + (h - ts.y) * 0.5f),
      kWhite,
      label);
  return clicked;
}
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
  ImGui::Dummy(ImVec2(ts.x, ts.y + 10.0f));
}

int StyledCombo(
    const char* id, const char* const* items, int count, int cur, float w) {
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

  int picked = -1;
  ImGui::SetNextItemWidth(w);
  if (ImGui::BeginCombo(id, items[cur])) {
    for (int i = 0; i < count; ++i) {
      const bool sel = (cur == i);
      if (ImGui::Selectable(items[i], sel)) {
        picked = i;
      }
      if (sel) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor(12);
  return picked;
}

void LanguageDropdown(float w) {
  static const char* kItems[] = {"中文", "English"};
  const int cur = (i18n::Current() == i18n::Lang::Zh) ? 0 : 1;
  const int picked = StyledCombo("##lang", kItems, 2, cur, w);
  if (picked >= 0) {
    i18n::Set(picked == 0 ? i18n::Lang::Zh : i18n::Lang::En);
  }
}

void ConfigSlotDropdown(float w) {
  const char* items[] = {
      T("配置 1", "Config 1"),
      T("配置 2", "Config 2"),
      T("配置 3", "Config 3")};
  const int picked =
      StyledCombo("##cfgslot", items, 3, ui::settings::Slot() - 1, w);
  if (picked >= 0) {
    ui::settings::SetSlot(picked + 1);
  }
}

struct MemInfo {
  double used_mb;
  double total_mb;
  float pct;
};

MemInfo QueryMemory() {
  MemInfo m{0.0, 0.0, 0.0f};

  uint64_t total = 0;
  size_t len = sizeof(total);
  int mib[2] = {CTL_HW, HW_MEMSIZE};
  if (sysctl(mib, 2, &total, &len, nullptr, 0) != 0 || total == 0) {
    return m;
  }

  mach_port_t host = mach_host_self();
  vm_size_t page_size = 0;
  if (host_page_size(host, &page_size) != KERN_SUCCESS || page_size == 0) {
    return m;
  }
  vm_statistics64_data_t vm;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  if (host_statistics64(
          host, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vm), &count) !=
      KERN_SUCCESS) {
    return m;
  }

  const uint64_t available =
      (static_cast<uint64_t>(vm.free_count) + vm.inactive_count +
       vm.purgeable_count) *
      page_size;
  const uint64_t used = (total > available) ? (total - available) : 0;

  constexpr double kMB = 1024.0 * 1024.0;
  m.total_mb = static_cast<double>(total) / kMB;
  m.used_mb = static_cast<double>(used) / kMB;
  m.pct = static_cast<float>(
      static_cast<double>(used) / static_cast<double>(total) * 100.0);
  return m;
}

void DrawMemoryMonitor(float w) {
  static MemInfo s_mem{0.0, 0.0, 0.0f};
  static double s_last = -1.0;
  const double now = ImGui::GetTime();
  if (s_last < 0.0 || now - s_last > 0.5) {
    s_mem = QueryMemory();
    s_last = now;
  }

  char left[48];
  std::snprintf(
      left, sizeof(left), "%.0f / %.0f MB", s_mem.used_mb, s_mem.total_mb);
  char right[16];
  std::snprintf(right, sizeof(right), "%.0f%%", s_mem.pct);

  ImFont* font = ImGui::GetFont();
  ImDrawList* dl = ImGui::GetWindowDrawList();
  constexpr float px = 15.0f;
  const ImVec2 p = ImGui::GetCursorScreenPos();
  const ImVec2 rs = font->CalcTextSizeA(px, FLT_MAX, 0.0f, right);
  dl->AddText(font, px, p, kWhite, left);
  dl->AddText(font, px, ImVec2(p.x + w - rs.x, p.y), AccentA(1.0f), right);
  ImGui::Dummy(ImVec2(w, px + 7.0f));

  const float frac = s_mem.pct * 0.01f;
  const ImU32 bar = (s_mem.pct >= 85.0f) ? IM_COL32(235, 70, 70, 255)
      : (s_mem.pct >= 65.0f)
      ? IM_COL32(240, 180, 60, 255)
      : ImGui::GetColorU32(theme::kAccent);
  const ImVec2 bp = ImGui::GetCursorScreenPos();
  constexpr float bh = 8.0f;
  dl->AddRectFilled(
      bp, ImVec2(bp.x + w, bp.y + bh), IM_COL32(255, 255, 255, 28), bh * 0.5f);
  if (frac > 0.0f) {
    const float fw = (frac < 1.0f) ? (w * frac) : w;
    dl->AddRectFilled(bp, ImVec2(bp.x + fw, bp.y + bh), bar, bh * 0.5f);
  }
  ImGui::Dummy(ImVec2(w, bh + 4.0f));
}

} // namespace

void Draw() {
  constexpr float pad = 20.0f;
  ImGui::Dummy(ImVec2(0, 14));
  ImGui::Indent(pad);
  const float avail = ImGui::GetContentRegionAvail().x - pad;

  Header(T("選項", "Options"));
  widgets::CircleCheck(
      "##chk_hiderec", T("過錄影", "Hide Record"), state::g_hide_record, avail);

  ImGui::Dummy(ImVec2(0, 14));
  Header(T("設定", "Settings"));
  ConfigSlotDropdown(avail);
  ImGui::Dummy(ImVec2(0, 8));
  static double s_saved_at = -10.0;
  const bool just_saved = ImGui::GetTime() - s_saved_at < 1.5;
  if (ActionButton(
          "##save_cfg",
          just_saved ? T("已儲存 ✓", "Saved ✓")
                     : T("儲存設定", "Save Settings"),
          avail)) {
    if (ui::settings::Save()) {
      s_saved_at = ImGui::GetTime();
    }
  }

  ImGui::Dummy(ImVec2(0, 14));
  Header(T("記憶體", "Memory"));
  DrawMemoryMonitor(avail);

  ImGui::Dummy(ImVec2(0, 14));
  Header(T("語言", "Language"));
  LanguageDropdown(avail < 200.0f ? avail : 200.0f);

  ImGui::Unindent(pad);
}

} // namespace ui::home_page
