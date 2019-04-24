#pragma once

#include <component/component.h>
#include <game/skinned_mesh_component.h>

namespace game {
using namespace component;
struct AnimationComponent {
  String current_animation;
  float current_animation_time;
  
  ComponentHandle2<SkinnedMeshComponent> skinned_mesh;
};
} // namespace game

declare_component(game::AnimationComponent)
