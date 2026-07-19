#pragma once

#include "imgui.h"

namespace ui {

inline void DrawBoldText(
    ImDrawList* dl,
    ImFont* font,
    float px,
    const ImVec2& pos,
    ImU32 col,
    const char* text) {
  dl->AddText(font, px, pos, col, text);
}

inline void DrawTextOutlined(
    ImDrawList* dl,
    ImFont* font,
    float px,
    const ImVec2& pos,
    ImU32 col,
    const char* text,
    ImU32 outline = IM_COL32(0, 0, 0, 255)) {
  for (int dy = -1; dy <= 1; ++dy) {
    for (int dx = -1; dx <= 1; ++dx) {
      if (dx == 0 && dy == 0) {
        continue;
      }
      dl->AddText(
          font,
          px,
          ImVec2(pos.x + (float)dx, pos.y + (float)dy),
          outline,
          text);
    }
  }
  dl->AddText(font, px, pos, col, text);
}

} // namespace ui
