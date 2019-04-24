#pragma once

#include <asset/asset.h>
#include <runtime/mesh.h>
#include <graphicsinterface/graphicsinterface.h>
#include <utils/helpers.h>
#include <array>

namespace renderer {
struct Resource;
}
struct aiAnimation;
struct aiNodeAnim;
struct aiNode;

namespace runtime {
static const i32 NUMBEROFBONES = 4;
struct AnimationInfo {
  String friendly_name;
  String filename;
  float duration;
  bool has_exit_time;
  void *ai_scene;
};

static const int MAX_BONES_PER_VERTEX = 4;
struct VertexBoneData {
  // std::array<unsigned int, MAX_BONES_PER_VERTEX> ids;
  std::array<float, MAX_BONES_PER_VERTEX> ids;
  std::array<float, MAX_BONES_PER_VERTEX> weights;

  void AddBoneData(unsigned int BoneID, float Weight);
};

struct sBoneInfo {
  glm::mat4 BoneOffset;
  glm::mat4 final_transformation;
  glm::mat4 ObjectBoneTransformation;
};

struct SkinnedMeshVertex {
  SkinnedMeshVertex()
      : x(0.0f)
      , y(0.0f)
      , z(0.0f)
      , w(1.0f)
      //, r(0.0f)
      //, g(0.0f)
      //, b(0.0f)
      //, a(1.0f)
      , // Note alpha is 1.0
      nx(0.0f)
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
    //#ifdef _DEBUG
    memset(this->bone_id, 0, sizeof(u32) * NUMBEROFBONES);
    memset(this->bone_weights, 0, sizeof(float) * NUMBEROFBONES);
    // So these are essentially this:
    //		unsigned int boneID[4];
    //		float boneWeights[4];
    //#endif // DEBUG
  };
  // Destructor isn't really needed here
  //~sVertex_xyz_rgba_n_uv2_bt_skinnedMesh();

  float x, y, z, w; // 16
  // float r, g, b, a;     // 32
  float nx, ny, nz, nw; // 48
  float u0, v0, u1, v1; // 60
  float tx, ty, tz, tw; // tangent				//
  float bx, by, bz, bw; // bi-normal			//
  // For the 4 bone skinned mesh information
  float bone_id[NUMBEROFBONES];      // New		//
  float bone_weights[NUMBEROFBONES]; // New		//

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

  struct AnimationState {
    struct StateDetails {
      StateDetails()
          : current_time(0.0f)
          , total_time(0.0f)
          , frame_step_time(0.0f){};
      String name;
      float current_time;    // Time (frame) in current animation
      float total_time;      // Total time animation goes
      float frame_step_time; // Number of seconds to 'move' the animation
      // Returns true if time had to be reset
      bool increment_time(bool reset_to_zero = true) {
        bool did_we_reset = false;

        this->current_time += this->frame_step_time;
        if (this->current_time >= this->total_time) {
          this->current_time = 0.0f;
          did_we_reset = true;
        }

        return did_we_reset;
      }
    };

    struct TempStuff {
      String current_animation;
      String previous_animation;
    };
    TempStuff temp;

    // Extent Values for skinned mesh
    glm::vec3 minXYZ_from_SM_Bones;
    glm::vec3 maxXYZ_from_SM_Bones;
    // Store all the bones for this model, buing updated per frame
    Vector<glm::mat4x4> object_bone_transforms;
    Vector<StateDetails> animation_queue;
    StateDetails active_animation;
    StateDetails default_animation;
  };

  AnimationState state;
  void *pScene = nullptr;

  float get_duration_seconds(String animation_name);
  float find_animation_total_time(String animation_name);
  void bone_transform(float time_in_seconds, String animation_name, Vector<glm::mat4> &final_transformation,
                      Vector<glm::mat4> &globals, Vector<glm::mat4> &offsets);

  void read_node_hierarchy(float animation_time, String animation_name, const aiNode *pNode,
                           const glm::mat4 &ParentTransformMatrix);
  const aiNodeAnim *SkinnedMesh::find_node_animation_channel(const aiAnimation *pAnimation, const String &boneName);

  void calculate_glm_interpolated_scaling(float animation_time, const aiNodeAnim *node_anim, glm::vec3 &out);
  Map<String, AnimationInfo> animation_name_to_pscene; 
  void calculate_glm_interpolated_rotation(float animation_time, const aiNodeAnim *node_anim, glm::quat &out);
  void calculate_glm_interpolated_position(float animation_time, const aiNodeAnim *node_anim, glm::vec3 &out);
  unsigned int find_rotation(float animation_time, const aiNodeAnim *node_anim);
  unsigned int find_position(float animation_time, const aiNodeAnim *node_anim);
  unsigned int find_scaling(float animation_time, const aiNodeAnim *node_anim);

  glm::mat4 global_inverse_transformation;
  i32 number_of_vertices = 0;
  Vector<VertexBoneData> vertex_bone_data;
  Map<String, unsigned int> bone_name_to_bone_index;
  Vector<sBoneInfo> bone_info;
  unsigned int number_of_bones = 0;

private:
  SharedPtr<SkinnedMeshResource> render_data;
};
} // namespace runtime
