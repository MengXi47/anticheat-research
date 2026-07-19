#pragma once

namespace gfx {

void Init(void *metal_device);
void *MxIconTexture();
void *BackgroundTexture();
void *HomeIcon();
void *VisionIcon();
void *TargetIcon();

struct LabelTex {
  void *tex;
  float w;
  float h;
};

LabelTex AcquireLabel(const char *utf8, float font_px);

void SweepLabelCache();

}  // namespace gfx
