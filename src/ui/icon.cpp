#include "ui/icon.h"

#include "state/texture.h"

namespace ui {

void DrawMxIcon(
    const ImVec2& pos,
    float diameter,
    float rounding,
    bool tinted,
    bool border) {
  const ImVec2 p_max{pos.x + diameter, pos.y + diameter};
  ImDrawList* dl = ImGui::GetWindowDrawList();

  if (void* tex = gfx::MxIconTexture()) {
    const ImU32 tint =
        tinted ? IM_COL32(180, 180, 180, 255) : IM_COL32(255, 255, 255, 255);
    dl->AddImageRounded(
        reinterpret_cast<ImTextureID>(tex),
        pos,
        p_max,
        ImVec2(0, 0),
        ImVec2(1, 1),
        tint,
        rounding,
        ImDrawFlags_RoundCornersAll);
    if (border) {
      dl->AddRect(
          pos,
          p_max,
          IM_COL32(255, 255, 255, 45),
          rounding,
          ImDrawFlags_RoundCornersAll,
          1.5f);
    }
    return;
  }
  dl->AddRectFilled(pos, p_max, IM_COL32(40, 44, 52, 255), rounding);
  if (border) {
    dl->AddRect(pos, p_max, IM_COL32(255, 255, 255, 60), rounding, 0, 1.5f);
  }
}

} // namespace ui
