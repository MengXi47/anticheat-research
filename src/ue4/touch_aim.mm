#import "ue4/touch_aim.h"

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#include <cmath>
#include <mutex>

#import "PTFakeMetaTouch.h"
#include "state/esp_state.h"

namespace ue4::touch_aim {

namespace {

std::mutex g_mtx;
bool g_active = false;
NSInteger g_point_id = 0;
float g_touch_x = 0.0f;
float g_touch_y = 0.0f;
double g_last_move = 0.0;

constexpr double kMoveThrottle = 0.008;
constexpr float kStartOffsetX = 100.0f;
constexpr float kArriveDist = 1.0f;
constexpr float kFovRadiusCoeff = 0.6f;
constexpr float kEdgePad = 5.0f;
constexpr NSInteger kMaxPointId = 10;
constexpr float kSpeedMin = 0.01f;
constexpr float kSpeedMax = 2.0f;

void DispatchTouch(NSInteger pid, float x, float y, UITouchPhase phase) {
  dispatch_async(dispatch_get_main_queue(), ^{
    [PTFakeMetaTouch fakeTouchId:pid
                        AtPoint:CGPointMake(x, y)
                 withTouchPhase:phase];
  });
}

void ReleaseLocked() {
  if (!g_active) {
    return;
  }
  DispatchTouch(g_point_id, g_touch_x, g_touch_y, UITouchPhaseEnded);
  g_active = false;
  g_point_id = 0;
}

} // namespace

void Steer(float tx, float ty, float cx, float cy, float speed) {
  std::lock_guard<std::mutex> lk(g_mtx);

  const float dx = tx - cx;
  const float dy = ty - cy;
  const float dist = std::sqrt(dx * dx + dy * dy);

  const float W = state::g_screen_w.load(std::memory_order_relaxed);
  const float H = state::g_screen_h.load(std::memory_order_relaxed);

  if (!g_active) {
    if (dist <= kArriveDist) {
      return;
    }
    const NSInteger pid = [PTFakeMetaTouch getAvailablePointId];
    if (pid <= 0 || pid > kMaxPointId) {
      return;
    }
    g_point_id = pid;
    g_touch_x = cx + kStartOffsetX;
    g_touch_y = cy;
    g_active = true;
    DispatchTouch(pid, g_touch_x, g_touch_y, UITouchPhaseBegan);
    return;
  }
  if (dist <= kArriveDist) {
    ReleaseLocked();
    return;
  }
  if (W > 0.0f && H > 0.0f) {
    const float fov_r = std::sqrt(W * W + H * H) * kFovRadiusCoeff;
    if (dist > fov_r) {
      ReleaseLocked();
      return;
    }
  }

  if (speed < kSpeedMin) {
    speed = kSpeedMin;
  } else if (speed > kSpeedMax) {
    speed = kSpeedMax;
  }
  g_touch_x += dx * speed;
  g_touch_y += dy * speed;

  if (W > 0.0f) {
    if (g_touch_x < 0.0f) {
      g_touch_x = 0.0f;
    } else if (g_touch_x > W) {
      g_touch_x = W;
    }
  }
  if (H > 0.0f) {
    if (g_touch_y < 0.0f) {
      g_touch_y = 0.0f;
    } else if (g_touch_y > H) {
      g_touch_y = H;
    }
  }
  if (W > 0.0f && H > 0.0f &&
      (g_touch_x <= kEdgePad || g_touch_x >= W - kEdgePad ||
       g_touch_y <= kEdgePad || g_touch_y >= H - kEdgePad)) {
    ReleaseLocked();
    return;
  }

  const double now = CACurrentMediaTime();
  if (now - g_last_move < kMoveThrottle) {
    return;
  }
  g_last_move = now;
  DispatchTouch(g_point_id, g_touch_x, g_touch_y, UITouchPhaseMoved);
}

void Release() {
  std::lock_guard<std::mutex> lk(g_mtx);
  ReleaseLocked();
}

} // namespace ue4::touch_aim
