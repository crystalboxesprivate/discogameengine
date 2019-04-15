#pragma once

#include <runtime/static_mesh.h>
#include <asset/factory.h>

struct StaticMeshFactory : public asset::Factory {
  virtual const char *get_filename_extensions() override {
    return "obj;fbx;ply";
  }

  virtual asset::AssetRef create(const String &filename) override {
    return asset::AssetRef(new runtime::StaticMesh);
  }

protected:
  virtual void load_asset_data(asset::Asset &asset);
};
