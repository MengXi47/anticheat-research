#pragma once

#include <atomic>

namespace state {

inline std::atomic<bool> g_aimbot{false};
inline std::atomic<int> g_aim_backend{1};
inline std::atomic<bool> g_aim_draw_fov{false};
inline std::atomic<float> g_aim_fov{100.0f};
inline std::atomic<float> g_aim_speed_mem{1.0f};
inline std::atomic<float> g_aim_speed_touch{3.0f};
inline std::atomic<float> g_aim_shotgun_speed{3.0f};
inline std::atomic<float> g_aim_distance{100.0f};
inline std::atomic<int> g_aim_target_bone{0};
inline std::atomic<int> g_aim_method{1};
inline std::atomic<bool> g_aim_ignore_knocked{false};
inline std::atomic<bool> g_aim_ignore_bot{false};
inline std::atomic<bool> g_aim_recoil_comp{false};
inline std::atomic<float> g_aim_recoil_smooth{1.0f};
inline std::atomic<bool> g_aim_prediction{true};
inline std::atomic<bool> g_no_recoil{false};

}  // namespace state
