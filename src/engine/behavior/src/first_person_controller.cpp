#include <behavior/first_person_controller.h>
#include <engine>
#include <component/component.h>
#include <game/camera_component.h>
#include <game/transform_component.h>
#include <input/input.h>
#include <app/app.h>
#include <window/window.h>

using namespace input;
using namespace game;
using namespace glm;

namespace behavior {
CameraComponent *current = nullptr;
ComponentHandle2<TransformComponent> transform;


void FirstPersonCamera::start() {
  yaw = 90.f;
}

void init_me() {
  if (current) {
    return;
  }
  auto &comps = component::get_vector_of_components<CameraComponent>();
  // assert(comps.size());
  if (!comps.slots.size())
    return;

  current = component::find_and_get_mut<CameraComponent>(comps.get_slot_id(0));
  transform = component::find<TransformComponent>(comps.get_slot_id(0));
}

void FirstPersonCamera::update(double delta_time) {
  init_me();

  update_rotation();
  update_movement();

  current->view_matrix = view_matrix();
  current->projection_matrix = projection_matrix(app::get().get_window().get_aspect_ratio());

  component::get_mut(transform)->position = position;
}

mat4 FirstPersonCamera::view_matrix() {
  return lookAt(position, position + camera_front, camera_up);
}

mat4 FirstPersonCamera::projection_matrix(const float aspect) {
  const float fov = 0.4f * 3.14f;
  const float far = 1000.f;
  const float near = .01f;
  return perspective(fov, aspect, near, far);
}

vec2 mouse_prev;
vec2 mouse;

static const float MOUSE_MOVE_THRESHOLD = 0.0001f;

void FirstPersonCamera::update_rotation() {
  mouse = input::mouse::get_position();
  vec2 axis = mouse - mouse_prev;
  mouse_prev = mouse;

  float offset_x = axis.x;
  float offset_y = axis.y;

  bool is_mouse_moving = abs(offset_x) > MOUSE_MOVE_THRESHOLD || abs(offset_y) > MOUSE_MOVE_THRESHOLD;
  if (!is_mouse_moving)
    return;

  float &xoffset = offset_x;
  float &yoffset = offset_y;

  float sensitivity = 0.05f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  front.x = cos(radians(yaw)) * cos(radians(pitch));
  front.y = sin(radians(pitch));
  front.z = sin(radians(yaw)) * cos(radians(pitch));
  camera_front = normalize(front);
  horizontal = normalize(cross(front, camera_up));
}
void FirstPersonCamera::update_movement() {
  float deltaTime = (f32)app::get().time.delta_seconds;

  bool is_left_clicked = input::keyboard::is_pressed(input::Key::LeftShift);

  float factor = deltaTime * (is_left_clicked ? movement_settings.run_multiplier : 1.f);

  factor *= movement_settings.forward_speed;

  glm::vec3 translation(0);

  if (input::keyboard::is_pressed(input::Key::W)) {
    translation += camera_front * factor;
  }

  if (input::keyboard::is_pressed(input::Key::S)) {
    translation -= camera_front * factor;
  }

  if (input::keyboard::is_pressed(input::Key::A))
    translation += horizontal * factor;

  if (input::keyboard::is_pressed(input::Key::D))
    translation -= horizontal * factor;

  vec3 up = cross(camera_front, horizontal);
  if (input::keyboard::is_pressed(input::Key::Q)) {
    translation += up * factor;
  }

  if (input::keyboard::is_pressed(input::Key::E)) {
    translation -= up * factor;
  }

  position += translation;
}

} // namespace behavior
