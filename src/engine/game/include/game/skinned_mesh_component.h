#pragma once

#include <game/primitive_component.h>
#include <runtime/skinned_mesh.h>
#include <string>

namespace game {
using namespace component;
struct SkinnedMeshComponent {

  struct Extents {
    glm::vec3 min, max;
  };
  asset::AssetHandle<runtime::SkinnedMesh> skinned_mesh;

  Extents extents;
  std::vector<glm::mat4x4> object_to_bone_transforms;
};
} // namespace component
declare_component(game::SkinnedMeshComponent)
