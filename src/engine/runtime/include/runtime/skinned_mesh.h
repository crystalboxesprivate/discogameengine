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
struct sAnimationInfo {
  std::string friendlyName;
  std::string fileName;
  float duration;
  bool bHasExitTime;
  // const aiScene *pAIScene;
  void *pAIScene;
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
  glm::mat4 FinalTransformation;
  glm::mat4 ObjectBoneTransformation;
};

struct sVertex_xyz_rgba_n_uv2_bt_4Bones {
  sVertex_xyz_rgba_n_uv2_bt_4Bones()
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
    memset(this->boneID, 0, sizeof(u32) * NUMBEROFBONES);
    memset(this->boneWeights, 0, sizeof(float) * NUMBEROFBONES);
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
  float boneID[NUMBEROFBONES];      // New		//
  float boneWeights[NUMBEROFBONES]; // New		//

  inline friend Archive &operator<<(Archive &archive, sVertex_xyz_rgba_n_uv2_bt_4Bones &vert) {
    archive.serialize(&vert, sizeof(sVertex_xyz_rgba_n_uv2_bt_4Bones));
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
  Vector<sVertex_xyz_rgba_n_uv2_bt_4Bones> vertices;

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
      std::string name;
      float current_time;    // Time (frame) in current animation
      float total_time;      // Total time animation goes
      float frame_step_time; // Number of seconds to 'move' the animation
      // Returns true if time had to be reset
      // (for checking to see if the animation has finished or not)
      // TODO: Deal with running the animation backwards, perhaps??
      bool increment_time(bool bResetToZero = true) {
        bool bDidWeReset = false;

        this->current_time += this->frame_step_time;
        if (this->current_time >= this->total_time) {
          this->current_time = 0.0f;
          bDidWeReset = true;
        }

        return bDidWeReset;
      }
    };

    struct TempStuff {
      std::string current_animation;
      std::string previous_animation;
    };
    TempStuff temp;

    // Extent Values for skinned mesh
    // These can be updated per frame, from the "update skinned mesh" call
    glm::vec3 minXYZ_from_SM_Bones;
    glm::vec3 maxXYZ_from_SM_Bones;
    // Store all the bones for this model, buing updated per frame
    std::vector<glm::mat4x4> object_bone_transforms;

    std::vector<StateDetails> animation_queue;
    StateDetails active_animation;
    StateDetails default_animation;
  };

  AnimationState state;
  void *pScene = nullptr;

  float get_duration_seconds(std::string animationName);
  float find_animation_total_time(std::string animationName);
  void bone_transform(float TimeInSeconds,
                      std::string animationName, // Now we can pick the animation
                      std::vector<glm::mat4> &FinalTransformation, std::vector<glm::mat4> &Globals,
                      std::vector<glm::mat4> &Offsets);

  void read_node_hierarchy(float AnimationTime, std::string animationName, const aiNode *pNode,
                         const glm::mat4 &ParentTransformMatrix);
  const aiNodeAnim *SkinnedMesh::find_node_animation_channel(const aiAnimation *pAnimation, const String& boneName);

  void calculate_glm_interpolated_scaling(float AnimationTime, const aiNodeAnim *pNodeAnim, glm::vec3 &out);
  std::map<std::string /*animation FRIENDLY name*/,
           sAnimationInfo> mapAnimationFriendlyNameTo_pScene; // Animations
  void calculate_glm_interpolated_rotation(float AnimationTime, const aiNodeAnim *pNodeAnim, glm::quat &out);
  void calculate_glm_interpolated_position(float AnimationTime, const aiNodeAnim *pNodeAnim, glm::vec3 &out);
  unsigned int find_rotation(float AnimationTime, const aiNodeAnim *pNodeAnim);
  unsigned int find_position(float AnimationTime, const aiNodeAnim *pNodeAnim);
  unsigned int find_scaling(float AnimationTime, const aiNodeAnim *pNodeAnim);

  glm::mat4 mGlobalInverseTransformation;
  i32 m_numberOfVertices = 0;
  Vector<VertexBoneData> vecVertexBoneData;
  Map<String /*BoneName*/, unsigned int /* BoneIndex */> m_mapBoneNameToBoneIndex; // mMapping;
  Vector<sBoneInfo> bone_info;
  unsigned int number_of_bones = 0; // mNums;
private:
  SharedPtr<SkinnedMeshResource> render_data;
};
} // namespace runtime
