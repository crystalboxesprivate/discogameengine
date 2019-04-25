#pragma once

#include <runtime/skinned_mesh.h>
#include <asset/factory.h>
#include <array>

struct aiScene;
struct aiMesh;


struct VertexBoneData {
  float ids[4] = {0, 0, 0, 0};
  float weights[4] = {0, 0, 0, 0};
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
  void load_bones(aiMesh *Mesh, Vector<VertexBoneData> &vertexBoneData, runtime::SkinnedMesh& mesh_sm) ;
  bool load_mesh_animation(const std::string &friendlyName, const std::string &filename, runtime::SkinnedMesh&mesh,
                         bool hasExitTime = false); // lodaing only Animation

private:
  const aiScene *p_scene = nullptr;


};
