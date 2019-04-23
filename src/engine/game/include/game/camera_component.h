#pragma once
#include <component/component.h>
#include <string>

namespace game {
using namespace component;
struct CameraComponent {
  float clipping_plane_near;
  float clipping_plane_far;
  float field_of_view;

  float speed = 100.f;
  // gets recalculated
  glm::mat4 view_matrix;
  glm::mat4 projection_matrix;
};
} // namespace game
declare_component(game::CameraComponent)
