#include "ui/settings.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <map>

#include "common/log/log.h"
#include "state/aim_state.h"
#include "state/app_state.h"
#include "state/esp_state.h"
#include "ui/i18n.h"

namespace ui::settings {

namespace {

struct BoolField {
  const char* key;
  std::atomic<bool>* p;
};
struct FloatField {
  const char* key;
  std::atomic<float>* p;
};
struct IntField {
  const char* key;
  std::atomic<int>* p;
};

const BoolField kBools[] = {
    {"esp", &state::g_esp},
    {"esp_line", &state::g_esp_line},
    {"esp_health", &state::g_esp_health},
    {"esp_skeleton", &state::g_esp_skeleton},
    {"esp_name", &state::g_esp_name},
    {"esp_distance", &state::g_esp_distance},
    {"esp_weapon", &state::g_esp_weapon},
    {"esp_hide_bots", &state::g_esp_hide_bots},
    {"esp_team_id", &state::g_esp_team_id},
    {"esp_show_count", &state::g_esp_show_count},
    {"esp_deathbox", &state::g_esp_deathbox},
    {"esp_loot", &state::g_esp_loot},
    {"esp_vehicle", &state::g_esp_vehicle},
    {"esp_show_unknown", &state::g_esp_show_unknown},
    {"aimbot", &state::g_aimbot},
    {"aim_draw_fov", &state::g_aim_draw_fov},
    {"aim_ignore_knocked", &state::g_aim_ignore_knocked},
    {"aim_ignore_bot", &state::g_aim_ignore_bot},
    {"aim_recoil_comp", &state::g_aim_recoil_comp},
    {"aim_prediction", &state::g_aim_prediction},
    {"no_recoil", &state::g_no_recoil},
    {"hide_record", &state::g_hide_record},
};

const FloatField kFloats[] = {
    {"esp_line_thickness", &state::g_esp_line_thickness},
    {"aim_fov", &state::g_aim_fov},
    {"aim_speed_mem", &state::g_aim_speed_mem},
    {"aim_speed_touch", &state::g_aim_speed_touch},
    {"aim_shotgun_speed", &state::g_aim_shotgun_speed},
    {"aim_distance", &state::g_aim_distance},
    {"aim_recoil_smooth", &state::g_aim_recoil_smooth},
};

const IntField kInts[] = {
    {"aim_target_bone", &state::g_aim_target_bone},
    {"aim_method", &state::g_aim_method},
    {"aim_backend", &state::g_aim_backend},
};

std::map<std::string, std::string> ParseFlat(const std::string& s) {
  std::map<std::string, std::string> m;
  std::size_t i = 0;
  while (i < s.size()) {
    const std::size_t kq = s.find('"', i);
    if (kq == std::string::npos) {
      break;
    }
    const std::size_t ke = s.find('"', kq + 1);
    if (ke == std::string::npos) {
      break;
    }
    const std::string key = s.substr(kq + 1, ke - kq - 1);
    const std::size_t colon = s.find(':', ke + 1);
    if (colon == std::string::npos) {
      break;
    }
    std::size_t vs = colon + 1;
    while (vs < s.size() && (s[vs] == ' ' || s[vs] == '\t')) {
      ++vs;
    }
    std::size_t ve = vs;
    while (ve < s.size() && s[ve] != ',' && s[ve] != '}' && s[ve] != '\n' &&
           s[ve] != '\r') {
      ++ve;
    }
    std::string val = s.substr(vs, ve - vs);
    while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) {
      val.pop_back();
    }
    m[key] = val;
    i = ve;
  }
  return m;
}

int g_slot = 1;

std::string ConfigDir() {
  const std::string log = mxlog::GetFilePath();
  if (!log.empty()) {
    const std::size_t slash = log.find_last_of('/');
    if (slash != std::string::npos) {
      return log.substr(0, slash + 1);
    }
  }
  return "";
}

std::string SlotMetaPath() {
  return ConfigDir() + "config_active";
}

} // namespace

std::string FilePath(int slot) {
  if (slot < 1 || slot > kSlotCount) {
    slot = 1;
  }
  return ConfigDir() + "config_" + std::to_string(slot) + ".json";
}

bool Save() {
  std::string j = "{\n";
  char line[160];
  for (const auto& f : kBools) {
    std::snprintf(
        line,
        sizeof(line),
        "  \"%s\": %s,\n",
        f.key,
        f.p->load(std::memory_order_relaxed) ? "true" : "false");
    j += line;
  }
  for (const auto& f : kFloats) {
    std::snprintf(
        line,
        sizeof(line),
        "  \"%s\": %g,\n",
        f.key,
        f.p->load(std::memory_order_relaxed));
    j += line;
  }
  for (const auto& f : kInts) {
    std::snprintf(
        line,
        sizeof(line),
        "  \"%s\": %d,\n",
        f.key,
        f.p->load(std::memory_order_relaxed));
    j += line;
  }
  std::snprintf(
      line,
      sizeof(line),
      "  \"lang\": %d,\n",
      i18n::Current() == i18n::Lang::En ? 1 : 0);
  j += line;
  for (int i = 0; i < ue4::kItemCategoryCount; ++i) {
    std::snprintf(
        line,
        sizeof(line),
        "  \"loot_cat_%d\": %s,\n",
        i,
        state::g_loot_cat[i].load(std::memory_order_relaxed)
            ? "true"
            : "false");
    j += line;
  }
  const std::size_t last = j.find_last_of(',');
  if (last != std::string::npos) {
    j.erase(last, 1);
  }
  j += "}\n";

  const std::string path = FilePath(g_slot);
  std::FILE* fp = std::fopen(path.c_str(), "w");
  if (fp == nullptr) {
    return false;
  }
  std::fwrite(j.data(), 1, j.size(), fp);
  std::fclose(fp);
  return true;
}

bool Load() {
  const std::string path = FilePath(g_slot);
  std::FILE* fp = std::fopen(path.c_str(), "r");
  if (fp == nullptr) {
    return false;
  }
  std::string buf;
  char chunk[1024];
  std::size_t n;
  while ((n = std::fread(chunk, 1, sizeof(chunk), fp)) > 0) {
    buf.append(chunk, n);
  }
  std::fclose(fp);

  const auto m = ParseFlat(buf);
  const auto find = [&](const char* k) -> const std::string* {
    const auto it = m.find(k);
    return it == m.end() ? nullptr : &it->second;
  };
  for (const auto& f : kBools) {
    if (const auto* v = find(f.key)) {
      f.p->store(*v == "true", std::memory_order_relaxed);
    }
  }
  for (const auto& f : kFloats) {
    if (const auto* v = find(f.key)) {
      f.p->store(
          static_cast<float>(std::atof(v->c_str())), std::memory_order_relaxed);
    }
  }
  for (const auto& f : kInts) {
    if (const auto* v = find(f.key)) {
      f.p->store(std::atoi(v->c_str()), std::memory_order_relaxed);
    }
  }
  if (const auto* v = find("lang")) {
    i18n::Set(std::atoi(v->c_str()) == 1 ? i18n::Lang::En : i18n::Lang::Zh);
  }
  if (const auto* v = find("aim_speed")) {
    state::g_aim_speed_mem.store(
        static_cast<float>(std::atof(v->c_str())), std::memory_order_relaxed);
  }
  for (int i = 0; i < ue4::kItemCategoryCount; ++i) {
    char k[24];
    std::snprintf(k, sizeof(k), "loot_cat_%d", i);
    if (const auto* v = find(k)) {
      state::g_loot_cat[i].store(*v == "true", std::memory_order_relaxed);
    }
  }
  return true;
}

int Slot() {
  return g_slot;
}

void SetSlot(int slot) {
  if (slot < 1 || slot > kSlotCount) {
    return;
  }
  g_slot = slot;
  if (std::FILE* fp = std::fopen(SlotMetaPath().c_str(), "w")) {
    std::fprintf(fp, "%d", slot);
    std::fclose(fp);
  }
  Load();
}

void Init() {
  if (std::FILE* fp = std::fopen(SlotMetaPath().c_str(), "r")) {
    int s = 0;
    if (std::fscanf(fp, "%d", &s) == 1 && s >= 1 && s <= kSlotCount) {
      g_slot = s;
    }
    std::fclose(fp);
  }
  Load();
}

} // namespace ui::settings
