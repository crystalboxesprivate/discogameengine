#pragma once

#include <component/component.h>
#include <game/skinned_mesh_component.h>

namespace game {
using namespace component;
struct AnimationComponent {
  String current_animation;
  float current_animation_time;

  struct StateDetails {
    StateDetails()
        : name(0)
        , current_time(0.0f)
        , total_time(0.0f)
        , frame_step_time(0.0f){};
    u64 name;
    float current_time;    // Time (frame) in current animation
    float total_time;      // Total time animation goes
    float frame_step_time; // Number of seconds to 'move' the animation

    bool increment_time(bool reset_to_zero = true) {
      bool did_we_reset = false;

      this->current_time += this->frame_step_time;
      if (this->current_time >= this->total_time) {
        this->current_time = 0.0f;
        did_we_reset = true;
      }

      return did_we_reset;
    }
  };

  // Extent Values for skinned mesh
  glm::vec3 minXYZ_from_SM_Bones;
  glm::vec3 maxXYZ_from_SM_Bones;
  // Store all the bones for this model, buing updated per frame
  Vector<glm::mat4x4> object_bone_transforms;
  Vector<StateDetails> animation_queue;
  StateDetails active_animation;
  StateDetails default_animation;

  ComponentHandle2<SkinnedMeshComponent> skinned_mesh;
};
} // namespace game

declare_component(game::AnimationComponent)
