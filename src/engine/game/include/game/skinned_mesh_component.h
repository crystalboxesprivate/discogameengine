#pragma once
#include <game/primitive_component.h>
// #include "cSimpleAssimpSkinnedMeshLoader_OneMesh.h"
#include <string>

namespace game {
using namespace component;
struct SkinnedMeshComponent /*: public PrimitiveComponent*/ {
  // AssimpSkinnedMesh mesh;

  struct Extents {
    glm::vec3 min, max;
  };
  Extents extents;
  std::vector<glm::mat4x4> object_to_bone_transforms;
};
} // namespace component
declare_component(game::SkinnedMeshComponent)
