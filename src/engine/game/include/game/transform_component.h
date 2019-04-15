#pragma once

#include <string>
#include <component/component.h>

namespace game {
using namespace component;
struct TransformComponent {
  TransformComponent()
      : position(glm::vec3(0))
      , scale3d(glm::vec3(1.f))
      , transform_matrix(glm::mat4(1.f)) {
    set_mesh_orientation_euler_angles(glm::vec3(0.f, 0.f, 0.f));
  }

  glm::vec3 position;
  glm::vec3 scale3d;
  glm::quat orient;
  // This value is automatically recalculated each frame.
  glm::mat4 transform_matrix;

  // This stuff should be decoupled from the "data" part.
  glm::vec3 get_mesh_orientation_euler_angles(bool is_degrees = false) const;
  glm::vec3 get_forward(glm::vec3 forwardModelSpace = glm::vec3(0.f, 0.f, 1.f)) const;

  void set_q_orientation(glm::quat newOrientation) {
    this->orient = newOrientation;
  }
  void set_uniform_scale(float scale);

  void set_mesh_orientation_euler_angles(glm::vec3 euler_angles, bool is_degrees = false);
  void set_mesh_orientation_euler_angles(float x, float y, float z, bool is_degrees = false);
  void adj_orientation_euler_angles(glm::vec3 adj_angle_euler, bool is_degrees = false);
  void adj_orientation_euler_angles(float x, float y, float z, bool is_degrees = false);
  void adj_mesh_orientation_q(glm::quat adj_orient_q);
};
} // namespace game
declare_component(game::TransformComponent)
