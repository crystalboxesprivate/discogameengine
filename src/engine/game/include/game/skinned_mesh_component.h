#pragma once

#include <game/primitive_component.h>
#include <runtime/skinned_mesh.h>
#include <game/transform_component.h>
#include <string>

namespace game {
using namespace component;
struct SkinnedMeshComponent {
  static const i32 MAX_NUMBER_OF_BONES = 100;
  struct MatrixBuffer {
    glm::mat4x4 bones[MAX_NUMBER_OF_BONES];
    glm::mat4x4 bones_previous[MAX_NUMBER_OF_BONES];
    // vec4 numBonesUsed;
  };

  component::ComponentHandle2<game::TransformComponent> cached_transform_component;
  asset::AssetHandle<runtime::SkinnedMesh> mesh;

  String current_animation;
  float animation_time;

  struct Extents {
    glm::vec3 min, max;
  };
  Extents extents;
  std::vector<glm::mat4x4> object_to_bone_transforms;
  std::vector<glm::mat4x4> bone_transforms;
  i32 number_of_bones_used;

  MatrixBuffer matrix_buffer;
};
} // namespace game
declare_component(game::SkinnedMeshComponent)
