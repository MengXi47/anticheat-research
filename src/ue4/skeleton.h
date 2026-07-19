#pragma once

#include <array>

#include "SDK/SDK.hpp"

namespace ue4::skeleton {

enum class Bone : int {
  Head,
  Neck,
  Spine03,
  Spine02,
  Spine01,
  Pelvis,
  UpperArmR,
  LowerArmR,
  HandR,
  UpperArmL,
  LowerArmL,
  HandL,
  ThighR,
  CalfR,
  FootR,
  ThighL,
  CalfL,
  FootL,
  Count,
};

inline constexpr int kBoneCount = static_cast<int>(Bone::Count);

struct Skeleton {
  std::array<SDK::FVector, kBoneCount> points{};
  bool valid = false;

  const SDK::FVector &At(Bone b) const { return points[static_cast<int>(b)]; }
};

void GetSkeleton(SDK::ASTExtraPlayerCharacter *character, Skeleton &out);

struct BoneLink {
  Bone a;
  Bone b;
};

const BoneLink *BoneLinks(int &out_count);

}  // namespace ue4::skeleton
