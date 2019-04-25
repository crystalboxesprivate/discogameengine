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
  // glm::mat4 bone_offset;
  glm::mat4 final_transformation;
  glm::mat4 object_bone_transformation;

  inline friend Archive &operator<<(Archive &archive, BoneInfo &info) {
    // archive << info.bone_offset;
    archive << info.final_transformation;
    archive << info.object_bone_transformation;
    return archive;
  }
};

struct SkinnedMeshVertex {
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

  inline const runtime::animation::Animation *get_animation(u64 index) const {
    return index >= animations.size() ? nullptr : &animations[index];
  }

  static void bone_transform(const SkinnedMesh& skinned_mesh, float time_in_seconds, u64 animation_name, Vector<glm::mat4> &final_transformation,
                      Vector<glm::mat4> &globals, Vector<glm::mat4> &offsets);

  // void read_node_hierarchy(Vector<glm::mat4> &transforms, Vector<glm::mat4> &object_bone_transforms, const Vector<glm::mat4> &offsets,
  //                          float animation_time, const runtime::animation::Animation &pAnimation,
  //                          const runtime::animation::Node &pNode, const glm::mat4 &parent_transform_matrix);

  u64 default_animation = 0;
  u32 number_of_bones = 0;
  Vector<glm::mat4> bone_offsets;

  double ticks_per_second;

  Vector<animation::Animation> animations;
  glm::mat4 global_inverse_transformation;

  runtime::animation::Node hierarchy;

private:
  SharedPtr<SkinnedMeshResource> render_data;
};
} // namespace runtime
