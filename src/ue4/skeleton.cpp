#include "ue4/skeleton.h"

#include <atomic>
#include <cstdint>

#include "common/log/log.h"
#include "common/ptr/valid.h"
#include "ue4/fname.h"

namespace ue4::skeleton {

namespace {

constexpr const char *kBoneNames[kBoneCount] = {
    "Head",
    "neck_01",
    "spine_03",
    "spine_02",
    "spine_01",
    "pelvis",
    "upperarm_r",
    "lowerarm_r",
    "hand_r",
    "upperarm_l",
    "lowerarm_l",
    "hand_l",
    "thigh_r",
    "calf_r",
    "foot_r",
    "thigh_l",
    "calf_l",
    "foot_l",
};

constexpr BoneLink kLinks[] = {
    {Bone::Head, Bone::Neck},
    {Bone::Neck, Bone::Spine03},
    {Bone::Spine03, Bone::Spine02},
    {Bone::Spine02, Bone::Spine01},
    {Bone::Spine01, Bone::Pelvis},
    {Bone::Neck, Bone::UpperArmR},
    {Bone::UpperArmR, Bone::LowerArmR},
    {Bone::LowerArmR, Bone::HandR},
    {Bone::Neck, Bone::UpperArmL},
    {Bone::UpperArmL, Bone::LowerArmL},
    {Bone::LowerArmL, Bone::HandL},
    {Bone::Pelvis, Bone::ThighR},
    {Bone::ThighR, Bone::CalfR},
    {Bone::CalfR, Bone::FootR},
    {Bone::Pelvis, Bone::ThighL},
    {Bone::ThighL, Bone::CalfL},
    {Bone::CalfL, Bone::FootL},
};

std::array<SDK::FName, kBoneCount> g_bone_fnames{};
std::atomic<bool> g_fnames_ok{false};

bool EnsureBoneFNames() {
  if (g_fnames_ok.load(std::memory_order_acquire)) {
    return true;
  }
  int unresolved = 0;
  for (int i = 0; i < kBoneCount; ++i) {
    const int32_t idx = ue4::FindFNameIndex(kBoneNames[i]);
    g_bone_fnames[i] = SDK::FName(idx);
    if (idx == 0) {
      ++unresolved;
    }
  }
  if (unresolved != 0) {
    return false;
  }
  g_fnames_ok.store(true, std::memory_order_release);
  return true;
}

}  // namespace

const BoneLink *BoneLinks(int &out_count) {
  out_count = static_cast<int>(std::size(kLinks));
  return kLinks;
}

void GetSkeleton(SDK::ASTExtraPlayerCharacter *character, Skeleton &out) {
  out.valid = false;
  if (!ptr::IsValid(character) || SDK::FName::GNames == nullptr) {
    return;
  }

  if (!EnsureBoneFNames()) {
    return;
  }

  const SDK::FVector zero{0.0f, 0.0f, 0.0f};
  for (int i = 0; i < kBoneCount; ++i) {
    out.points[i] = character->GetBonePos(g_bone_fnames[i], zero);
  }
  out.valid = true;
}

}  // namespace ue4::skeleton
