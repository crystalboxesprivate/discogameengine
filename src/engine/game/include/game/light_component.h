#pragma once
#include <component/component.h>
#include <string>

namespace game {
using namespace component;
enum class LightType : u8 { Point, Spot, Directional };
struct LightComponent {
  LightComponent()
      : type(LightType::Point)
      , diffuse_alpha(0.92f, 0.43f, 0.10f, 1.0f)
      , attenuation(0.0f, 0.0016f, 0.00002f, -431602080.0f)
      , is_on(true) {
  }
  LightType type;
  glm::vec4 color;
  float intensity;

  glm::vec4 attenuation;
  glm::vec4 diffuse_alpha;
  bool is_on;
  m_serialize_basic_type(LightType);
};
} // namespace game
declare_component(game::LightComponent)
