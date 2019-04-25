#pragma once

#include <engine>

namespace runtime {
namespace animation {
enum class AnimationBehaviour : u8 { Default, Constant, Linear, Repeat };
struct VectorKey {
  double time;
  glm::vec3 value;

  inline friend Archive &operator<<(Archive &archive, VectorKey &vector_key) {
    archive << vector_key.time;
    archive << vector_key.value;
    return archive;
  }
};

struct QuatKey {
  double time;
  glm::quat value;

  inline friend Archive &operator<<(Archive &archive, QuatKey &quat_key) {
    archive << quat_key.time;
    archive << quat_key.value;
    return archive;
  }
};

struct MeshKey {
  double time;
  u32 value;

  inline friend Archive &operator<<(Archive &archive, MeshKey &mesh_key) {
    archive << mesh_key.time;
    archive << mesh_key.value;
    return archive;
  }
};

struct Channel {
  String node_name;

  Vector<VectorKey> position_keys;
  Vector<QuatKey> rotation_keys;
  Vector<VectorKey> scaling_keys;

  AnimationBehaviour pre_state;
  AnimationBehaviour post_state;

  inline friend Archive &operator<<(Archive &archive, Channel &channel) {
    archive << channel.node_name;
    archive << channel.position_keys;
    archive << channel.rotation_keys;
    archive << channel.scaling_keys;

    archive.serialize(&channel.pre_state, sizeof(AnimationBehaviour));
    archive.serialize(&channel.post_state, sizeof(AnimationBehaviour));
    return archive;
  }
};

struct MeshChannel {
  String name;
  Vector<MeshKey> keys;

  inline friend Archive &operator<<(Archive &archive, MeshChannel &mesh_channel) {
    archive << mesh_channel.name;
    archive << mesh_channel.keys;
    return archive;
  }
};

struct Animation {
  String name;
  f64 duration = 0.0;
  f64 ticks_per_second = 0.0;

  f64 duration_seconds = 0.0;

  Vector<Channel> channels;
  Vector<MeshChannel> mesh_channels;

  inline friend Archive &operator<<(Archive &archive, Animation &anim) {
    archive << anim.name;
    archive << anim.duration;
    archive << anim.ticks_per_second;
    archive << anim.duration_seconds;
    archive << anim.channels;
    archive << anim.mesh_channels;
    return archive;
  }
};

struct Node {
  String name;
  i32 bone_index = -1;

  glm::mat4 transform;
  Vector<Node> children;

  inline friend Archive &operator<<(Archive &archive, Node &node) {
    archive << node.name;
    archive << node.bone_index;
    archive.serialize(&node.transform, sizeof(glm::mat4));
    archive << node.children;
    return archive;
  }
};

u32 find_rotation(float animation_time, const Channel &node_anim);
u32 find_position(float animation_time, const Channel &node_anim);
u32 find_scaling(float animation_time, const Channel &node_anim);
void calculate_glm_interpolated_rotation(float animation_time, const Channel &node_anim, glm::quat &out);
void calculate_glm_interpolated_position(float animation_time, const Channel &node_anim, glm::vec3 &out);
void calculate_glm_interpolated_scaling(float animation_time, const Channel &node_anim, glm::vec3 &out);

} // namespace animation
} // namespace runtime