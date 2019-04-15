#include <game/transform_component.h>
using namespace game;
using namespace glm;

// void TransformComponent::serialize(Archive &archive) {
//   Super::serialize(archive);
//   archive << position;
//   archive << scale3d;
//   archive << orient;
// }

void TransformComponent::set_mesh_orientation_euler_angles(vec3 euler_angles, bool is_degrees) {
  if (is_degrees)
    euler_angles = vec3(radians(euler_angles.x), radians(euler_angles.y), radians(euler_angles.z));
  this->orient = quat(vec3(euler_angles.x, euler_angles.y, euler_angles.z));
}

void TransformComponent::set_mesh_orientation_euler_angles(float x, float y, float z, bool is_degrees) {
  return this->set_mesh_orientation_euler_angles(vec3(x, y, z), is_degrees);
}

vec3 TransformComponent::get_mesh_orientation_euler_angles(bool is_degrees) const {
  vec3 rot = eulerAngles(this->orient);
  if (is_degrees)
    rot = vec3(degrees(rot.x), degrees(rot.y), degrees(rot.z));
  return rot;
}

void TransformComponent::adj_orientation_euler_angles(vec3 adj_angle_euler, bool is_degrees) {
  if (is_degrees) {
    adj_angle_euler = vec3(radians(adj_angle_euler.x), radians(adj_angle_euler.y), radians(adj_angle_euler.z));
  }
  // Step 1: make a quaternion that represents the angle we want to rotate
  quat rotationAdjust(adj_angle_euler);
  // Step 2: Multiply this quaternion by the existing quaternion. This "adds" the angle we want.
  this->orient *= rotationAdjust;
  this->orient = this->orient * rotationAdjust;
}

void TransformComponent::adj_orientation_euler_angles(float x, float y, float z, bool is_degrees) {
  return this->adj_orientation_euler_angles(vec3(x, y, z), is_degrees);
}

void TransformComponent::adj_mesh_orientation_q(quat adj_orient_q) {
  this->orient *= adj_orient_q;
}

vec3 TransformComponent::get_forward(vec3 forward_model_space) const {
  const vec4 forward_vec4 = vec4(0.f, 0.f, 1.f, 1.f);
  const mat4 orientation_matrix = mat4(quat(orient.x, orient.y, orient.z, orient.w));
  vec4 forward_dir_ws = orientation_matrix * forward_vec4;
  // optional normalize
  forward_dir_ws = normalize(forward_dir_ws);
  vec3 forward_vector(forward_dir_ws.x, forward_dir_ws.y, forward_dir_ws.z);
  return forward_vector;
}

void TransformComponent::set_uniform_scale(float scale) {
  this->scale3d = vec3(scale, scale, scale);
}
