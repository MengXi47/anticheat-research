#pragma once

#include <atomic>
#include <cfloat>
#include <cstdio>

#include "imgui.h"
#include "ui/text_fx.h"
#include "ui/theme.h"

namespace ui::widgets {

inline void CircleCheck(
    const char* id, const char* label, std::atomic<bool>& flag, float w) {
  constexpr float kRowH = 36.0f;
  constexpr float kR = 12.0f;
  const ImU32 white = IM_COL32(255, 255, 255, 255);

  const ImVec2 p0 = ImGui::GetCursorScreenPos();
  const float cy = p0.y + kRowH * 0.5f;
  const ImVec2 c(p0.x + kR + 2.0f, cy);

  constexpr float kHitPad = 6.0f;
  const float hit = (kR + kHitPad) * 2.0f;
  ImGui::SetCursorScreenPos(ImVec2(c.x - kR - kHitPad, c.y - kR - kHitPad));
  ImGui::InvisibleButton(id, ImVec2(hit, hit));
  const bool hovered = ImGui::IsItemHovered();
  if (ImGui::IsItemClicked()) {
    flag.store(
        !flag.load(std::memory_order_relaxed), std::memory_order_relaxed);
  }
  const bool on = flag.load(std::memory_order_relaxed);

  ImGui::SetCursorScreenPos(p0);
  ImGui::Dummy(ImVec2(w, kRowH));

  ImDrawList* dl = ImGui::GetWindowDrawList();
  if (on) {
    dl->AddCircleFilled(c, kR, ImGui::GetColorU32(theme::kAccent), 28);
    dl->AddLine(
        ImVec2(c.x - kR * 0.42f, c.y + kR * 0.02f),
        ImVec2(c.x - kR * 0.05f, c.y + kR * 0.34f),
        white,
        2.4f);
    dl->AddLine(
        ImVec2(c.x - kR * 0.05f, c.y + kR * 0.34f),
        ImVec2(c.x + kR * 0.46f, c.y - kR * 0.34f),
        white,
        2.4f);
  } else {
    dl->AddCircleFilled(c, kR, ImGui::GetColorU32(theme::kCheckOff), 28);
    dl->AddCircle(
        c,
        kR,
        hovered ? IM_COL32(255, 255, 255, 110) : IM_COL32(255, 255, 255, 40),
        28,
        1.3f);
  }

  ImFont* font = ImGui::GetFont();
  const ImVec2 ts =
      font->CalcTextSizeA(theme::kToggleTextPx, FLT_MAX, 0.0f, label);
  DrawBoldText(
      dl,
      font,
      theme::kToggleTextPx,
      ImVec2(c.x + kR + 12.0f, cy - ts.y * 0.5f),
      white,
      label);
}

inline bool LineSlider(
    const char* id,
    float* v,
    float vmin,
    float vmax,
    const char* fmt,
    float w) {
  constexpr float kRowH = 30.0f;
  constexpr float kTrackH = 4.0f;
  constexpr float kHandleR = 10.0f;
  constexpr float kValueW = 56.0f;
  constexpr float kValuePx = 16.0f;
  const ImU32 accent = ImGui::GetColorU32(theme::kAccent);
  const ImU32 track_off = IM_COL32(255, 255, 255, 40);
  const ImU32 white = IM_COL32(255, 255, 255, 255);

  const ImVec2 p0 = ImGui::GetCursorScreenPos();
  const bool show_value = fmt != nullptr && w > kValueW + kHandleR * 4.0f;
  const float track_w = show_value ? (w - kValueW) : w;

  ImGui::InvisibleButton(id, ImVec2(track_w, kRowH));
  const bool active = ImGui::IsItemActive();
  const bool hovered = ImGui::IsItemHovered();

  const float x0 = p0.x + kHandleR;
  const float x1 = p0.x + track_w - kHandleR;
  const float cy = p0.y + kRowH * 0.5f;
  const float span = (x1 > x0) ? (x1 - x0) : 1.0f;

  bool changed = false;
  if (active) {
    float t = (ImGui::GetIO().MousePos.x - x0) / span;
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    const float nv = vmin + t * (vmax - vmin);
    if (nv != *v) {
      *v = nv;
      changed = true;
    }
  }

  float t = (vmax > vmin) ? (*v - vmin) / (vmax - vmin) : 0.0f;
  t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
  const float hx = x0 + t * span;

  ImDrawList* dl = ImGui::GetWindowDrawList();
  const float half = kTrackH * 0.5f;
  dl->AddRectFilled(
      ImVec2(x0, cy - half), ImVec2(x1, cy + half), track_off, half);
  dl->AddRectFilled(ImVec2(x0, cy - half), ImVec2(hx, cy + half), accent, half);
  const float r = (active || hovered) ? kHandleR + 1.0f : kHandleR;
  dl->AddCircleFilled(ImVec2(hx, cy), r, accent, 24);
  dl->AddCircle(ImVec2(hx, cy), r, white, 24, 2.0f);

  if (show_value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), fmt, *v);
    ImFont* font = ImGui::GetFont();
    const ImVec2 ts = font->CalcTextSizeA(kValuePx, FLT_MAX, 0.0f, buf);
    DrawBoldText(
        dl,
        font,
        kValuePx,
        ImVec2(p0.x + w - ts.x, cy - ts.y * 0.5f),
        white,
        buf);
  }

  return changed;
}

} // namespace ui::widgets
