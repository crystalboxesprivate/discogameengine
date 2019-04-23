#pragma once
// #include <Interfaces/iRigidBody.h>
#include <component/component.h>
#include <string>

#include <game/transform_component.h>

namespace game {
using namespace component;
enum class RigidBodyType : u8 { Plane, Sphere, Cylinder, Box, Unknown };
inline Archive &operator<<(Archive &archive, RigidBodyType &type) {
  archive.serialize(&type, sizeof(RigidBodyType));
  return archive;
}
struct RigidBodyComponent {
  RigidBodyType type;
  glm::vec3 half_extents;

  float plane_constant;
  glm::vec3 plane_normal;
  glm::vec3 pivot;
  glm::vec3 pivot_b;
  glm::vec3 axis;
  glm::vec3 offset;

  float position_simulation_weight = 1.f;

  i32 axis_id;
  float mass;
  float radius = 10.f;

  ComponentHandle2<TransformComponent> transform;
  void *native_physics_ptr = nullptr;

  // run-time values
  bool had_collision;
  float inverse_mass;
};
} // namespace game
declare_component(game::RigidBodyComponent)
