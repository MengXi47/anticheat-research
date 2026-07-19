#include "ui/esp.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <vector>

#include "imgui.h"
#include "state/aim_state.h"
#include "state/esp_buffer.h"
#include "state/esp_state.h"
#include "state/item_buffer.h"
#include "state/texture.h"
#include "ue4/items_table.h"
#include "ue4/skeleton.h"
#include "ui/i18n.h"
#include "ui/text_fx.h"

namespace esp {

namespace {

constexpr int kHeadBone = static_cast<int>(ue4::skeleton::Bone::Head);

constexpr ImU32 kBoneVisible = IM_COL32(0, 255, 0, 230);
constexpr ImU32 kBoneBlocked = IM_COL32(255, 80, 80, 200);

constexpr int kStaleMs = 500;
constexpr float kBarWidth = 84.0f;
constexpr float kBarHeight = 14.0f;
constexpr float kChevron = 5.0f;
constexpr float kFrameThick = 1.5f;
constexpr float kHeadGapRatio = 0.10f;
constexpr float kHeadGapMin = 6.0f;
constexpr float kHeadGapMax = 44.0f;
constexpr float kHeadGap = 26.0f;
constexpr float kNamePx = 9.0f;
constexpr float kLabelPx = 12.0f;
constexpr float kLabelGap = 1.0f;
constexpr float kLineOriginY = 0.0f;

constexpr ImU32 kFrameRed = IM_COL32(255, 0, 0, 255);
constexpr ImU32 kHealthFill = IM_COL32(255, 0, 0, 140);
constexpr ImU32 kBotFrame = IM_COL32(0, 255, 0, 255);
constexpr ImU32 kBotFill = IM_COL32(0, 255, 0, 140);
constexpr ImU32 kNameColor = IM_COL32(255, 255, 255, 255);
constexpr ImU32 kTeamColor = IM_COL32(60, 150, 255, 255);
constexpr ImU32 kDistColor = IM_COL32(255, 215, 40, 255);
constexpr ImU32 kWeaponColor = IM_COL32(255, 170, 60, 255);
constexpr float kWeaponPx = 14.0f;
constexpr float kWeaponGap = 6.0f;
constexpr ImU32 kCountHuman = IM_COL32(110, 225, 90, 255);
constexpr ImU32 kCountBot = IM_COL32(180, 186, 196, 255);

float BarHeadGap(const state::Player& p) {
  float min_y = FLT_MAX;
  float max_y = -FLT_MAX;
  for (const auto& pt : p.points) {
    if (!pt.onscreen) {
      continue;
    }
    min_y = std::min(min_y, pt.pos.y);
    max_y = std::max(max_y, pt.pos.y);
  }
  if (max_y < min_y) {
    return kHeadGap;
  }
  return std::clamp((max_y - min_y) * kHeadGapRatio, kHeadGapMin, kHeadGapMax);
}

constexpr ImU32 kKnockedFill = IM_COL32(255, 255, 0, 111);
constexpr ImU32 kKnockedFrame = IM_COL32(255, 255, 255, 255);
constexpr float kCircleBarGap = 2.0f;

struct HeadCircle {
  bool ok = false;
  float radius = 0.0f;
  ImVec2 center{};
};

HeadCircle ComputeHeadCircle(const state::Player& p) {
  const auto& head = p.points[kHeadBone];
  if (!head.onscreen) {
    return {};
  }
  const auto& foot_r = p.points[static_cast<int>(ue4::skeleton::Bone::FootR)];
  const auto& foot_l = p.points[static_cast<int>(ue4::skeleton::Bone::FootL)];
  const auto& pelvis = p.points[static_cast<int>(ue4::skeleton::Bone::Pelvis)];
  float lower_y;
  if (foot_r.onscreen) {
    lower_y = foot_r.pos.y;
  } else if (foot_l.onscreen) {
    lower_y = foot_l.pos.y;
  } else if (pelvis.onscreen) {
    lower_y = pelvis.pos.y;
  } else {
    return {};
  }
  const float div = (foot_r.onscreen || foot_l.onscreen) ? 10.0f : 5.0f;
  const float r = std::fabs(head.pos.y - lower_y) / div;
  if (r < 1.0f) {
    return {};
  }
  HeadCircle hc;
  hc.ok = true;
  hc.radius = r;
  hc.center = ImVec2(head.pos.x, head.pos.y + r * -0.4f);
  return hc;
}

float BarBottomY(const state::Player& p, bool skeleton_on) {
  if (skeleton_on) {
    const HeadCircle hc = ComputeHeadCircle(p);
    if (hc.ok) {
      return (hc.center.y - hc.radius) - kCircleBarGap;
    }
  }
  return p.points[kHeadBone].pos.y - BarHeadGap(p);
}

void DrawPlayerInfo(ImDrawList* dl, const state::Player& p) {
  const auto& head = p.points[kHeadBone];
  if (!head.onscreen) {
    return;
  }
  const bool show_health = state::g_esp_health.load(std::memory_order_relaxed);
  const bool show_name = state::g_esp_name.load(std::memory_order_relaxed);
  const bool show_team = state::g_esp_team_id.load(std::memory_order_relaxed);
  const bool show_dist = state::g_esp_distance.load(std::memory_order_relaxed);
  const bool show_weapon = state::g_esp_weapon.load(std::memory_order_relaxed);
  if (!show_health && !show_name && !show_team && !show_dist && !show_weapon) {
    return;
  }

  ImFont* font = ImGui::GetFont();

  const bool show_skeleton =
      state::g_esp_skeleton.load(std::memory_order_relaxed);
  const float cx = head.pos.x;
  const float bottom = BarBottomY(p, show_skeleton);
  const float top = bottom - kBarHeight;
  const float mid = (bottom + top) * 0.5f;
  const float lx = cx - kBarWidth * 0.5f;
  const float rx = cx + kBarWidth * 0.5f;

  if (show_health) {
    const ImU32 fill_col =
        p.knocked ? kKnockedFill : (p.is_bot ? kBotFill : kHealthFill);
    const ImU32 frame_col =
        p.knocked ? kKnockedFrame : (p.is_bot ? kBotFrame : kFrameRed);
    float ratio = 0.0f;
    if (p.health_max > 0.0f) {
      ratio = std::clamp(p.health / p.health_max, 0.0f, 1.0f);
    }
    if (ratio > 0.0f) {
      const float fr = lx + kBarWidth * ratio;
      const ImVec2 fill[6] = {
          {lx, bottom},
          {fr, bottom},
          {fr + kChevron, mid},
          {fr, top},
          {lx, top},
          {lx - kChevron, mid},
      };
      dl->AddConvexPolyFilled(fill, 6, fill_col);
    }
    const ImVec2 frame[6] = {
        {lx, bottom},
        {rx, bottom},
        {rx + kChevron, mid},
        {rx, top},
        {lx, top},
        {lx - kChevron, mid},
    };
    dl->AddPolyline(frame, 6, frame_col, kFrameThick, ImDrawFlags_Closed);
  }
  if (show_name && !p.name.empty()) {
    const gfx::LabelTex lt = gfx::AcquireLabel(p.name.c_str(), kNamePx);
    if (lt.tex) {
      const ImVec2 tp(cx - lt.w * 0.5f, mid - lt.h * 0.5f);
      dl->AddImage(
          reinterpret_cast<ImTextureID>(lt.tex),
          tp,
          ImVec2(tp.x + lt.w, tp.y + lt.h),
          ImVec2(0, 0),
          ImVec2(1, 1),
          kNameColor);
    } else {
      const ImVec2 ts =
          font->CalcTextSizeA(kNamePx, FLT_MAX, 0.0f, p.name.c_str());
      const ImVec2 tp(cx - ts.x * 0.5f, mid - ts.y * 0.5f);
      dl->AddText(font, kNamePx, tp, kNameColor, p.name.c_str());
    }
  }

  const float label_y = top - kLabelGap - kLabelPx;
  if (show_team) {
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%d", p.team_id);
    ui::DrawTextOutlined(
        dl, font, kLabelPx, ImVec2(lx, label_y), kTeamColor, buf);
  }
  if (show_dist) {
    char buf[24];
    std::snprintf(buf, sizeof(buf), "[%.0fM]", p.distance);
    const ImVec2 ts = font->CalcTextSizeA(kLabelPx, FLT_MAX, 0.0f, buf);
    ui::DrawTextOutlined(
        dl, font, kLabelPx, ImVec2(rx - ts.x, label_y), kDistColor, buf);
  }

  if (show_weapon && p.weapon_name != nullptr && p.weapon_name[0] != '\0') {
    float max_x = -FLT_MAX;
    float max_y = -FLT_MAX;
    bool any = false;
    for (const auto& pt : p.points) {
      if (!pt.onscreen) {
        continue;
      }
      any = true;
      max_x = std::max(max_x, pt.pos.x);
      max_y = std::max(max_y, pt.pos.y);
    }
    if (any) {
      const ImVec2 tp(max_x + kWeaponGap, max_y);
      ui::DrawTextOutlined(
          dl, font, kWeaponPx, tp, kWeaponColor, p.weapon_name);
    }
  }
}

void DrawCountCard(ImDrawList* dl, int humans, int bots) {
  ImFont* font = ImGui::GetFont();
  constexpr float kPx = 16.0f;
  char buf_h[40];
  char buf_b[40];
  std::snprintf(
      buf_h, sizeof(buf_h), "%s %d", ui::i18n::T("玩家", "Players"), humans);
  std::snprintf(
      buf_b, sizeof(buf_b), "%s %d", ui::i18n::T("人機", "Bots"), bots);

  const ImVec2 sh = font->CalcTextSizeA(kPx, FLT_MAX, 0.0f, buf_h);
  const ImVec2 sb = font->CalcTextSizeA(kPx, FLT_MAX, 0.0f, buf_b);
  constexpr float kGap = 18.0f; // 兩段之間
  constexpr float kPadX = 16.0f;
  constexpr float kPadY = 7.0f;
  const float text_h = std::max(sh.y, sb.y);
  const float card_w = sh.x + kGap + sb.x + kPadX * 2.0f;
  const float card_h = text_h + kPadY * 2.0f;
  const float sw = state::g_screen_w.load(std::memory_order_relaxed);
  const float x0 = sw * 0.5f - card_w * 0.5f;
  constexpr float y0 = 12.0f;

  dl->AddRectFilled(
      ImVec2(x0, y0),
      ImVec2(x0 + card_w, y0 + card_h),
      IM_COL32(14, 16, 20, 205),
      8.0f);
  dl->AddRect(
      ImVec2(x0, y0),
      ImVec2(x0 + card_w, y0 + card_h),
      IM_COL32(255, 255, 255, 45),
      8.0f);

  const float ty = y0 + kPadY;
  const float tx = x0 + kPadX;
  dl->AddText(font, kPx, ImVec2(tx, ty), kCountHuman, buf_h);
  dl->AddText(font, kPx, ImVec2(tx + sh.x + kGap, ty), kCountBot, buf_b);
}

void DrawFovCircle() {
  if (!state::g_aim_draw_fov.load(std::memory_order_relaxed)) {
    return;
  }
  const float w = state::g_screen_w.load(std::memory_order_relaxed);
  const float h = state::g_screen_h.load(std::memory_order_relaxed);
  const float r = state::g_aim_fov.load(std::memory_order_relaxed);
  if (w <= 0.0f || h <= 0.0f || r <= 0.0f) {
    return;
  }
  ImGui::GetBackgroundDrawList()->AddCircle(
      ImVec2(w * 0.5f, h * 0.5f), r, IM_COL32(255, 255, 255, 90), 64, 1.5f);
}

ImU32 ItemColor(int category) {
  if (category == state::kCategoryVehicle) {
    return IM_COL32(180, 120, 255, 255);
  }
  if (category == state::kCategoryUnknownItem) {
    return IM_COL32(255, 255, 255, 255);
  }
  if (category < 0) {
    return IM_COL32(255, 215, 40, 255);
  }
  switch (static_cast<ue4::ItemCategory>(category)) {
    case ue4::ItemCategory::Rifle:
    case ue4::ItemCategory::SMG:
    case ue4::ItemCategory::Sniper:
    case ue4::ItemCategory::Shotgun:
    case ue4::ItemCategory::LMG:
    case ue4::ItemCategory::Pistol:
    case ue4::ItemCategory::SpecialWeapon:
    case ue4::ItemCategory::Melee:
      return IM_COL32(255, 170, 60, 255);
    case ue4::ItemCategory::Attachment:
      return IM_COL32(120, 200, 255, 255);
    case ue4::ItemCategory::Ammo:
      return IM_COL32(200, 200, 200, 255);
    case ue4::ItemCategory::Ghillie:
      return IM_COL32(150, 255, 150, 255);
    case ue4::ItemCategory::Armor:
      return IM_COL32(90, 160, 255, 255);
    case ue4::ItemCategory::Consumable:
      return IM_COL32(120, 235, 120, 255);
    case ue4::ItemCategory::Throwable:
      return IM_COL32(255, 110, 90, 255);
    case ue4::ItemCategory::RareLoot:
      return IM_COL32(255, 105, 180, 255);
    case ue4::ItemCategory::Event:
      return IM_COL32(120, 255, 220, 255);
    default:
      return IM_COL32(220, 220, 220, 255);
  }
}

void DrawItems(ImDrawList* dl) {
  if (!state::g_esp_deathbox.load(std::memory_order_relaxed) &&
      !state::g_esp_loot.load(std::memory_order_relaxed) &&
      !state::g_esp_vehicle.load(std::memory_order_relaxed)) {
    return;
  }
  if (!state::item_buffer::IsFresh(kStaleMs)) {
    return;
  }
  const auto items = state::item_buffer::Snapshot();
  if (!items || items->empty()) {
    return;
  }
  constexpr float kMarkR = 5.0f;
  constexpr float kItemPx = 11.0f;
  ImFont* font = ImGui::GetFont();
  for (const auto& it : *items) {
    if (!it.onscreen) {
      continue;
    }
    const ImU32 col = ItemColor(it.category);
    const ImVec2 c = it.pos;
    const ImVec2 diamond[4] = {
        {c.x, c.y - kMarkR},
        {c.x + kMarkR, c.y},
        {c.x, c.y + kMarkR},
        {c.x - kMarkR, c.y}};
    dl->AddPolyline(diamond, 4, col, ImDrawFlags_Closed, 1.5f);
    char buf[48];
    if (it.name) {
      std::snprintf(buf, sizeof(buf), "%s [%.0fM]", it.name, it.distance);
    } else {
      std::snprintf(buf, sizeof(buf), "ID:%d [%.0fM]", it.id, it.distance);
    }
    const ImVec2 ts = font->CalcTextSizeA(kItemPx, FLT_MAX, 0.0f, buf);
    ui::DrawTextOutlined(
        dl,
        font,
        kItemPx,
        ImVec2(c.x - ts.x * 0.5f, c.y + kMarkR + 2.0f),
        col,
        buf);
  }
}

} // namespace

void Draw() {
  DrawFovCircle();

  if (!state::g_esp.load(std::memory_order_relaxed)) {
    return;
  }

  DrawItems(ImGui::GetBackgroundDrawList());

  if (!state::esp_buffer::IsFresh(kStaleMs)) {
    return;
  }

  const auto frame = state::esp_buffer::Snapshot();
  if (!frame || frame->empty()) {
    return;
  }

  ImDrawList* dl = ImGui::GetBackgroundDrawList();
  const float thickness =
      state::g_esp_line_thickness.load(std::memory_order_relaxed);

  const bool show_skeleton =
      state::g_esp_skeleton.load(std::memory_order_relaxed);
  const bool show_line = state::g_esp_line.load(std::memory_order_relaxed);
  const bool hide_bots = state::g_esp_hide_bots.load(std::memory_order_relaxed);
  const ImVec2 line_origin(
      state::g_screen_w.load(std::memory_order_relaxed) * 0.5f, kLineOriginY);
  int link_count = 0;
  const auto* links = ue4::skeleton::BoneLinks(link_count);
  int humans = 0;
  int bots = 0;
  for (const auto& sk : *frame) {
    if (sk.is_bot) {
      ++bots;
    } else {
      ++humans;
    }

    if (hide_bots && sk.is_bot) {
      continue;
    }

    if (show_line) {
      const auto& head = sk.points[kHeadBone];
      if (head.onscreen) {
        const ImVec2 bar_top(
            head.pos.x, BarBottomY(sk, show_skeleton) - kBarHeight);
        dl->AddLine(line_origin, bar_top, sk.color, thickness);
      }
    }

    if (show_skeleton) {
      for (int e = 0; e < link_count; ++e) {
        const auto& a = sk.points[static_cast<int>(links[e].a)];
        const auto& b = sk.points[static_cast<int>(links[e].b)];
        if (a.onscreen && b.onscreen) {
          const ImU32 col =
              (a.visible && b.visible) ? kBoneVisible : kBoneBlocked;
          dl->AddLine(a.pos, b.pos, col, thickness);
        }
      }

      const HeadCircle hc = ComputeHeadCircle(sk);
      if (hc.ok) {
        const auto& head_pt = sk.points[kHeadBone];
        const ImU32 head_col = head_pt.visible ? kBoneVisible : kBoneBlocked;
        dl->AddCircle(hc.center, hc.radius, head_col, 0, thickness);
      }
    }

    DrawPlayerInfo(dl, sk);
  }

  if (state::g_esp_show_count.load(std::memory_order_relaxed)) {
    DrawCountCard(dl, humans, bots);
  }
}

} // namespace esp
