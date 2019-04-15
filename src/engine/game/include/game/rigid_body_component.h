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
struct RigidBodyComponent : public Component {
  declare_component(RigidBodyComponent);
  RigidBodyComponent()
      : velocity(glm::vec3(0.0f))
      , acceleration(glm::vec3(0.0f)) /* , rigid_body(nullptr) */ {
  }
  // virtual void serialize(Archive &archive) override {
  //   Super::serialize(archive);

  //   archive << type;

  //   archive << plane_constant;
  //   archive << plane_normal;
  //   archive << half_extents;
  //   archive << pivot;
  //   archive << pivot_b;
  //   archive << axis;
  //   archive << offset;
  //   archive << axis_id;
  //   archive << mass;
  //   archive << inverse_mass;
  //   archive << radius;

  //   archive << transform;
  // }

  RigidBodyType type;
  float plane_constant;
  glm::vec3 plane_normal;
  glm::vec3 half_extents;
  glm::vec3 pivot;
  glm::vec3 pivot_b;
  glm::vec3 axis;
  glm::vec3 offset;

  i32 axis_id;
  float mass;
  float radius = 10.f;

  ComponentHandle<game::TransformComponent> transform;

  // run-time values
  glm::vec3 velocity;
  glm::vec3 acceleration;
  bool had_collision;
  float inverse_mass;
};
} // namespace game
