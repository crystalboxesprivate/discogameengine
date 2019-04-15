#pragma once

#include <behavior/behavior.h>
#include <core/math/math.h>

namespace behavior {
struct FirstPersonCamera : public Behavior {
  FirstPersonCamera()
      : position(0, 0, -2.f) {
  }
  glm::vec3 position;
  struct ViewSettings {
    bool is_smoothed = false;
    float smooth_time = 5.f;
    float sensitivity_x = 0.05f;
    float sensitivity_y = .05f;
    float min_x = -90.f;
    float max_x = 90.f;
  };
  struct MovementSettings {
    float forward_speed = 56.f;
    float backward_speed = 32.f;
    float strafe_speed = 20.f;
    float run_multiplier = 5.f;
  };

  MovementSettings movement_settings;

  glm::vec3 front = glm::vec3(0);
  glm::vec3 horizontal = glm::vec3(0);
  glm::vec3 camera_up = glm::vec3(0, 1, 0);
  glm::vec3 camera_front = glm::vec3(0, 0, 1);

  float yaw = 0.f;
  float pitch = 0.f;

  glm::mat4 view_matrix();
  glm::mat4 projection_matrix(const float aspect);


  virtual void start() override;
  virtual void update(double delta_time) override;

private:
  void update_rotation();
  void update_movement();
};
} // namespace behavior
