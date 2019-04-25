#include <runtime/animation.h>
using namespace glm;
namespace runtime {
namespace animation {

u32 find_rotation(float animation_time, const runtime::animation::Channel &node_anim) {
  for (u32 rotation_key_index = 0; rotation_key_index < node_anim.rotation_keys.size() - 1; rotation_key_index++) {
    if (animation_time < (float)node_anim.rotation_keys[rotation_key_index + 1].time) {
      return rotation_key_index;
    }
  }

  return 0;
}

u32 find_position(float animation_time, const runtime::animation::Channel &node_anim) {
  for (u32 PositionKeyIndex = 0; PositionKeyIndex < node_anim.position_keys.size() - 1; PositionKeyIndex++) {
    if (animation_time < (float)node_anim.position_keys[PositionKeyIndex + 1].time) {
      return PositionKeyIndex;
    }
  }
  return 0;
}

u32 find_scaling(float animation_time, const runtime::animation::Channel &node_anim) {
  for (u32 scaling_key_index = 0; scaling_key_index < node_anim.scaling_keys.size() - 1; scaling_key_index++) {
    if (animation_time < (float)node_anim.scaling_keys[scaling_key_index + 1].time) {
      return scaling_key_index;
    }
  }

  return 0;
}

void calculate_glm_interpolated_rotation(float animation_time, const runtime::animation::Channel &node_anim,
                                         quat &out) {
  if (node_anim.rotation_keys.size() == 1) {
    out.w = node_anim.rotation_keys[0].value.w;
    out.x = node_anim.rotation_keys[0].value.x;
    out.y = node_anim.rotation_keys[0].value.y;
    out.z = node_anim.rotation_keys[0].value.z;
    return;
  }

  u32 rotation_index = find_rotation(animation_time, node_anim);
  u32 next_rotation_index = (rotation_index + 1);
  assert(next_rotation_index < node_anim.rotation_keys.size());
  float delta_time =
      (float)(node_anim.rotation_keys[next_rotation_index].time - node_anim.rotation_keys[rotation_index].time);
  float factor = (animation_time - (float)node_anim.rotation_keys[rotation_index].time) / delta_time;

  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;

  const quat &start_glm = node_anim.rotation_keys[rotation_index].value;
  const quat &end_glm = node_anim.rotation_keys[next_rotation_index].value;

  out = slerp(start_glm, end_glm, factor);
  out = normalize(out);

  return;
}

void calculate_glm_interpolated_position(float animation_time, const runtime::animation::Channel &node_anim,
                                         vec3 &out) {
  if (node_anim.position_keys.size() == 1) {
    out.x = node_anim.position_keys[0].value.x;
    out.y = node_anim.position_keys[0].value.y;
    out.z = node_anim.position_keys[0].value.z;
    return;
  }

  u32 position_index = find_position(animation_time, node_anim);
  u32 next_position_index = (position_index + 1);
  assert(next_position_index < node_anim.position_keys.size());
  float delta_time =
      (float)(node_anim.position_keys[next_position_index].time - node_anim.position_keys[position_index].time);
  float factor = (animation_time - (float)node_anim.position_keys[position_index].time) / delta_time;
  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;
  const vec3 &start = node_anim.position_keys[position_index].value;
  const vec3 &end = node_anim.position_keys[next_position_index].value;
  out = (end - start) * factor + start;
  return;
}
void calculate_glm_interpolated_scaling(float animation_time, const runtime::animation::Channel &node_anim, vec3 &out) {
  if (node_anim.scaling_keys.size() == 1) {
    out.x = node_anim.scaling_keys[0].value.x;
    out.y = node_anim.scaling_keys[0].value.y;
    out.z = node_anim.scaling_keys[0].value.z;
    return;
  }

  u32 scaling_index = find_scaling(animation_time, node_anim);
  u32 next_scaling_index = (scaling_index + 1);
  assert(next_scaling_index < node_anim.scaling_keys.size());
  float delta_time =
      (float)(node_anim.scaling_keys[next_scaling_index].time - node_anim.scaling_keys[scaling_index].time);
  float factor = (animation_time - (float)node_anim.scaling_keys[scaling_index].time) / delta_time;

  if (factor < 0.0f)
    factor = 0.0f;
  if (factor > 1.0f)
    factor = 1.0f;

  const vec3 &start = node_anim.scaling_keys[scaling_index].value;
  const vec3 &end = node_anim.scaling_keys[next_scaling_index].value;

  out = (end - start) * factor + start;
  return;
}

} // namespace animation
} // namespace runtime