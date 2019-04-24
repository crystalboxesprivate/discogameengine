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
  f64 duration;
  f64 ticks_per_second;
  Vector<Channel> channels;
  Vector<MeshChannel> mesh_channels;

  inline friend Archive &operator<<(Archive &archive, Animation &anim) {
    archive << anim.name;
    archive << anim.duration;
    archive << anim.ticks_per_second;
    archive << anim.channels;
    archive << anim.mesh_channels;
    return archive;
  }
};

struct Node {
  String name;
  glm::mat4 transform;
  Vector<Node> children;
};

struct Scene {
  Node root_node;
  double ticks_per_second;
};
} // namespace animation
} // namespace runtime
