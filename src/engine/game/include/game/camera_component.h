#pragma once
#include <component/component.h>
#include <string>

namespace game {
using namespace component;
struct CameraComponent : public Component {
  enum class LightType : u8 { Point, Spot, Directional };
  declare_component(CameraComponent);
  CameraComponent()
      : speed(100.f) {
  }
  float speed;
  // gets recalculated
  glm::mat4 view_matrix;
  glm::mat4 projection_matrix;
};
} // namespace game
