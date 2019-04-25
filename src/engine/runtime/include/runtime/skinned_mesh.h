#pragma once

#include <asset/asset.h>
#include <runtime/mesh.h>
#include <graphicsinterface/graphicsinterface.h>
#include <utils/helpers.h>
#include <array>

#include <runtime/animation.h>

namespace renderer {
struct Resource;
}

struct aiAnimation;
struct aiNodeAnim;
struct aiNode;

namespace runtime {
static const int NUMBER_OF_BONES = 4;

struct BoneInfo {
  glm::mat4 bone_offset;
  glm::mat4 final_transformation;
  glm::mat4 object_bone_transformation;
};

struct SkinnedMeshVertex {
  SkinnedMeshVertex()
      : x(0.0f)
      , y(0.0f)
      , z(0.0f)
      , w(1.0f)
      , nx(0.0f)
      , ny(0.0f)
      , nz(0.0f)
      , nw(1.0f)
      , u0(0.0f)
      , v0(0.0f)
      , u1(0.0f)
      , v1(0.0f)
      , tx(0.0f)
      , ty(0.0f)
      , tz(0.0f)
      , tw(1.0f)
      , bx(0.0f)
      , by(0.0f)
      , bz(0.0f)
      , bw(1.0f) {
    memset(this->bone_id, 0, sizeof(u32) * NUMBER_OF_BONES);
    memset(this->bone_weights, 0, sizeof(float) * NUMBER_OF_BONES);
  };

  float x, y, z, w;
  float nx, ny, nz, nw;
  float u0, v0, u1, v1;
  float tx, ty, tz, tw;
  float bx, by, bz, bw;
  float bone_id[NUMBER_OF_BONES];
  float bone_weights[NUMBER_OF_BONES];

  inline friend Archive &operator<<(Archive &archive, SkinnedMeshVertex &vert) {
    archive.serialize(&vert, sizeof(SkinnedMeshVertex));
    return archive;
  }
};

struct SkinnedMeshResource {
  SkinnedMeshResource();
  graphicsinterface::VertexStream vertex_stream;
  graphicsinterface::IndexBufferRef index_buffer;
  graphicsinterface::VertexBufferRef vertex_buffer;
};

struct SkinnedMesh : public asset::Asset {
  declare_asset_type(SkinnedMesh);
  SkinnedMesh() {
  }

  Vector<i32> indices;
  Vector<SkinnedMeshVertex> vertices;

  virtual void free() override {
    utils::free_vector(indices);
    utils::free_vector(vertices);
  }

  SkinnedMeshResource *get_render_resource();
  virtual void serialize(Archive &archive) override;

  math::Box bounds;

  float get_duration_seconds(u64 animation_name);
  float find_animation_total_time(u64 animation_name);
  void bone_transform(float time_in_seconds, u64 animation_name, Vector<glm::mat4> &final_transformation,
                      Vector<glm::mat4> &globals, Vector<glm::mat4> &offsets);

  void read_node_hierarchy(float animation_time, u64 animation_name, const runtime::animation::Node &pNode,
                           const glm::mat4 &parent_transform_matrix);

  const runtime::animation::Channel *SkinnedMesh::find_node_animation_channel(runtime::animation::Animation &animation,
                                                                              const String &boneName);

  u64 default_animation = 0;
  double ticks_per_second;

  
  Vector <animation::Animation> animation_name_to_pscene;

  glm::mat4 global_inverse_transformation;

  runtime::animation::Node hierarchy;
  Map<u64, u32> bone_name_to_bone_index;
  Vector<BoneInfo> bone_info;
  u32 number_of_bones = 0;

private:
  SharedPtr<SkinnedMeshResource> render_data;
};
} // namespace runtime
