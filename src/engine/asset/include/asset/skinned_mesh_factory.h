#pragma once

#include <runtime/skinned_mesh.h>
#include <asset/factory.h>
#include <array>

struct aiScene;
struct aiMesh;
static const int MAX_BONES_PER_VERTEX = 4;
struct VertexBoneData {
  // std::array<unsigned int, MAX_BONES_PER_VERTEX> ids;
  std::array<float, MAX_BONES_PER_VERTEX> ids;
  std::array<float, MAX_BONES_PER_VERTEX> weights;

  void AddBoneData(unsigned int BoneID, float Weight);
};

struct sAnimationInfo {
  std::string friendlyName;
  std::string fileName;
  float duration;
  bool bHasExitTime;
  const aiScene *pAIScene;
};

struct sBoneInfo {
  glm::mat4 BoneOffset;
  glm::mat4 FinalTransformation;
  glm::mat4 ObjectBoneTransformation;
};
struct SkinnedMeshFactory : public asset::Factory {
  virtual const char *get_filename_extensions() override {
    return "skinnedmeshjson";
  }

  virtual asset::AssetRef create(const String &filename) override {
    return asset::AssetRef(new runtime::SkinnedMesh);
  }

protected:
  virtual void load_asset_data(asset::Asset &asset);
  void LoadBones(aiMesh *Mesh, Vector<VertexBoneData> &vertexBoneData);
  bool LoadMeshAnimation(const std::string &friendlyName, const std::string &filename,
                         bool hasExitTime = false); // lodaing only Animation

private:
  glm::mat4 mGlobalInverseTransformation;
  i32 m_numberOfVertices;
  Vector<VertexBoneData> vecVertexBoneData;
  const aiScene *pScene = nullptr;

  Map<String /*BoneName*/, unsigned int /* BoneIndex */> m_mapBoneNameToBoneIndex; // mMapping;
  Vector<sBoneInfo> mBoneInfo;
  unsigned int mNumBones; // mNums;

  std::map<std::string /*animation FRIENDLY name*/,
           sAnimationInfo> mapAnimationFriendlyNameTo_pScene; // Animations
};
